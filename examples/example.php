<?php

include __DIR__ . '/../src/LAC.php';
include __DIR__ . '/../src/ThuLAC.php';

$lac = \FastFFI\LAC\LAC::new();

var_dump(
    $lac->parse("LAC智能中文分词库")
);


$thuLac = \FastFFI\LAC\ThuLAC::new();

var_dump(
    $thuLac->parse("thulac智能中文分词库")
);

