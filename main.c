#include <stdio.h>
#include "lac_c.h"

void run();

int main() {
    run();
}

void run() {
    const char *input = "LAC是个优秀的分词工具真得吗";

    CLac *lac;

    lac = new_lac("../model/lac_model");

    LacResult* lacResult = parse(input, lac);

    printf("words = %s\n", lacResult->words);
    printf("tags  = %s\n", lacResult->tags);
    free_result(lacResult);
    free_lac(lac);
}
