<?php

include __DIR__ . '/../src/LAC.php';

$lac = \FastFFI\LAC\LAC::new();

var_dump(
    $lac->parse("LAC智能中文分词库")
);
