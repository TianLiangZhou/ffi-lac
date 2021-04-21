//
// Created by meshell on 2021/4/20.
//

#include <iostream>
#include "lac.h"

int main() {

    LAC lac("../model/lac_model", CODE_UTF8);

    auto result = lac.run("上周末，捷克最新加入“驱逐战”，宣布驱逐18名俄罗斯使馆人员。俄方随即发出警告并进行反制，18日，俄罗斯方面宣布，捷克驻俄罗斯大使馆的20名外交人员为“不受欢迎的人士”，必须在19日结束前离开该国。");

    for (const auto& re : result) {
        std::cout << re.word << " ";
        std::cout << re.ftag << " ";
        std::cout << re.weight << " ";
        std::cout << re.tag << std::endl;
    }

}