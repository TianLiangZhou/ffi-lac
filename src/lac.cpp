/* Copyright (c) 2020 Baidu, Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <iostream>
#include <string.h>
#include <numeric>
#include "lac.h"
#include "lac_util.h"
#include "lac_custom.h"


/* LAC构造函数：初始化、装载模型和词典 */
LAC::LAC(const std::string& model_path, CODE_TYPE type)
    : _codetype(type),
      _lod(std::vector<std::vector<size_t> >(1)),
      _id2label_dict(new std::unordered_map<int64_t, std::string>),
      _q2b_dict(new std::unordered_map<std::string, std::string>),
      _word2id_dict(new std::unordered_map<std::string, int64_t>),
      custom(nullptr)
{

    size_t pos = model_path.find_last_of("\\/");
    auto prefix = (std::string::npos == pos)
                  ? ""
                  : model_path.substr(0, pos);
    auto last_path_name = model_path.substr(pos);

    this->_is_rank = last_path_name.find("rank") != std::string::npos;
    this->_model_parent = prefix;
    // 装载词典
    if (!this->_is_rank) {
        std::string word_dict_path = model_path + "/conf/word.dic";
        load_word2id_dict(word_dict_path, *_word2id_dict);

        // word_rep_dict_path
        std::string q2b_dict_path = model_path + "/conf/q2b.dic";
        load_q2b_dict(q2b_dict_path, *_q2b_dict);

        std::string label_dict_path = model_path + "/conf/tag.dic";
        load_id2label_dict(label_dict_path, *_id2label_dict);
    }

    // 使用AnalysisConfig装载模型，会进一步优化模型
    this->_place = paddle_infer::PlaceType::kCPU;
    paddle_infer::Config config;
    // config.SwitchIrOptim(false);       // 关闭优化
    config.SetModel(model_path + "/model");
    config.DisableGpu();
    config.DisableGlogInfo();
    config.SetCpuMathLibraryNumThreads(1);
    config.SwitchUseFeedFetchOps(false);
    this->_predictor = paddle_infer::CreatePredictor(config);

    // 初始化输入输出变量
    auto input_names = this->_predictor->GetInputNames();
    this->_input_tensor = this->_predictor->GetInputHandle(input_names[0]);
    auto output_names = this->_predictor->GetOutputNames();
    this->_output_tensor = this->_predictor->GetOutputHandle(output_names[0]);
    this->_oov_id = this->_word2id_dict->size() - 1;
    auto word_iter = this->_word2id_dict->find("OOV");
    if (word_iter != this->_word2id_dict->end())
    {
        this->_oov_id = word_iter->second;
    }

//    for (const auto& a : *_id2label_dict) {
//        std::cout << a.first  << " === " << a.second << std::endl;
//    }
}

/* 装载用户词典 */
int LAC::load_customization(const std::string& filename){
    /* 多线程热加载时容易出问题，多个线程共享custom
    if (custom){
        return custom->load_dict(filename);
    }
    */
    custom = std::make_shared<Customization>(filename);
    return 0;
}

/* 将字符串输入转为Tensor */
int LAC::feed_data(const std::vector<std::string> &querys)
{
    this->_seq_words_batch.clear();
    this->_lod[0].clear();

    this->_lod[0].push_back(0);
    int shape = 0;
    for (const auto & query : querys)
    {
        split_words(query, this->_codetype, this->_seq_words);
        this->_seq_words_batch.push_back(this->_seq_words);
        shape += this->_seq_words.size();
//        std::cout << "shape  = " << shape << std::endl;
        this->_lod[0].push_back(shape);
    }
    this->_input_tensor->SetLoD(this->_lod);
    this->_input_tensor->Reshape({shape, 1});
    std::vector<int64_t> input_d;
    for (auto & i : this->_seq_words_batch)
    {
        for (auto word : i)
        {
            // normalization
            auto q2b_iter = this->_q2b_dict->find(word);
            if (q2b_iter != this->_q2b_dict->end())
            {
                word = q2b_iter->second;
            }
//            std::cout << word << "  ";
            // get word_id
            int64_t word_id = this->_oov_id;
            auto word_iter = this->_word2id_dict->find(word);
            if (word_iter != this->_word2id_dict->end())
            {
                word_id = word_iter->second;
            }
            input_d.push_back(word_id);
//            std::cout << word_id << "  ";
        }
    }
    this->_words_length = input_d.size();
    this->_input_tensor->CopyFromCpu(input_d.data());
    return 0;
}

/* 对输出的标签进行解码转换为模型输出格式 */
int LAC::parse_targets(
    const std::vector<std::string> &tags,
    const std::vector<std::string> &words,
    std::vector<OutputItem> &result)
{
    result.clear();
    for (size_t i = 0; i < tags.size(); ++i)
    {
        // 若新词，则push_back一个新词，否则append到上一个词中
        if (result.empty() || tags[i].rfind('B') == tags[i].length() - 1 || tags[i].rfind('S') == tags[i].length() - 1)
        {
            OutputItem output_item;
            output_item.word = words[i];
            output_item.tag = tags[i].substr(0, tags[i].length() - 2);
            output_item.ftag = tags[i];
            output_item.weight = -1;
            result.push_back(output_item);
        }
        else
        {
            result[result.size() - 1].word += words[i];
        }
    }
    return 0;
}

std::vector<OutputItem> LAC::run(const std::string &query)
{
    std::vector<std::string> query_vector = std::vector<std::string>({query});
    auto result = run(query_vector);
    if (!result.empty()) {
        return result[0];
    }
    return {};
}

std::vector<std::vector<OutputItem>> LAC::run(const std::vector<std::string> &querys)
{
    LAC *lac = nullptr;
    std::vector<std::vector<OutputItem>> result;
    if (this->_is_rank) {
        lac = new LAC(this->_model_parent + "/lac_model", _codetype);
        result = lac->run(querys);
        auto input_tensor = lac->_input_tensor.release();
        auto names = this->_predictor->GetInputNames();
        auto entity_input_tensor = this->_predictor->GetInputHandle(names[1]);
        auto output_tensor = lac->_output_tensor.release();

        this->_input_tensor->SetLoD(input_tensor->lod());
        this->_input_tensor->Reshape(input_tensor->shape());
        std::vector<int64_t> data(lac->_words_length);
        input_tensor->CopyToCpu(data.data());
        this->_input_tensor->CopyFromCpu(data.data());
        entity_input_tensor->SetLoD(output_tensor->lod());
        entity_input_tensor->Reshape(output_tensor->shape());

        std::vector<int> output_shape = output_tensor->shape();
        int out_num = std::accumulate(output_shape.begin(), output_shape.end(), 1, std::multiplies<int>());
        std::vector<int64_t> out_data;
        out_data.resize(out_num);
        output_tensor->CopyToCpu(out_data.data());
        entity_input_tensor->CopyFromCpu(out_data.data());
    } else {
        this->feed_data(querys);
    }
    this->_predictor->Run();

    std::vector<int64_t> output_d;
    std::vector<int> rank_shape = this->_output_tensor->shape();
    int rank_out_num = std::accumulate(rank_shape.begin(), rank_shape.end(), 1, std::multiplies<int>());
    output_d.resize(rank_out_num);
    this->_output_tensor->CopyToCpu(output_d.data());

    if (lac != nullptr) {
        std::vector<int64_t> weight;
        for (size_t i = 0, e = lac->_labels.size(); i != e; i++) {
            if (lac->_labels[i].rfind('B') != std::string::npos || lac->_labels[i].rfind('S') != std::string::npos) {
                weight.push_back(output_d[i]);
                continue;
            }
            if (!weight.empty()) {
                weight[weight.size() - 1] = std::max(weight[weight.size() - 1], output_d[i]);
            }
        }
        for (size_t i = 0, c = result.size(); i != c; i++) {
            for (size_t j = 0, e = result[i].size(); j != e; j++) {
                result[i][j].weight = weight[j];
            }
        }
        return result;
    } else {
        this->_labels.clear();
        this->_results_batch.clear();
//        std::cout << "size  = " << this->_lod[0].size() << std::endl;

        for (size_t i = 0; i < this->_lod[0].size() - 1; ++i) {
            for (size_t j = 0; j < _lod[0][i + 1] - _lod[0][i]; ++j) {
                int64_t cur_label_id = output_d[_lod[0][i] + j];
                auto it = this->_id2label_dict->find(cur_label_id);
                this->_labels.push_back(it->second);
            }
            // 装载了用户干预词典，先进行干预处理
            if (custom) {
                custom->parse_customization(this->_seq_words_batch[i], this->_labels);
            }
            parse_targets(this->_labels, this->_seq_words_batch[i], this->_results);
//            this->_labels.clear();
            this->_results_batch.push_back(this->_results);
        }
        return this->_results_batch;
    }
}

extern "C" CLac new_lac(const char *mode) {
    std::string model_path(mode);
    // 装载模型和用户词典
    return (LAC*) new LAC(model_path, CODE_UTF8);
}

extern "C" void free_lac(CLac lac) {
    delete (LAC*) lac;
}
extern "C" void free_result(LacResult *lac) {
    delete lac;
}

extern "C" LacResult * parse(const char *input, CLac lac) {
    std::string str(input);
    auto result = ((LAC *)lac)->run(str);
    std::string words;
    std::string tags;
    std::string d = "-";
    std::string weight;
    for (auto & item : result){
        if(item.tag.length() > 0){
            tags.append(item.tag + " ");
        } else {
            tags.append(d + " ");
        }
        words.append(item.word + " ");
        weight.append(std::to_string(item.weight) + " ");
    }
    return new LacResult{
        strdup(words.c_str()),
        strdup(tags.c_str()),
        strdup(weight.c_str())
    };
}
