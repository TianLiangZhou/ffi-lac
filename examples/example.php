<?php

include __DIR__ . '/../src/LAC.php';

$lac = \FastFFI\LAC\LAC::new();

var_dump(
    $lac->parse("百度是一个高科技公司")
);
