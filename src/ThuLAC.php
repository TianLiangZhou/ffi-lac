<?php
namespace FastFFI\LAC;

use FFI;
use RuntimeException;

class ThuLAC
{
    /**
     * @var ?ThuLAC
     */
    private static ?ThuLAC $cc = null;

    /**
     * @var FFI
     */
    protected FFI $ffi;

    /**
     * @var string
     */
    protected string $dictDir = __DIR__ . '/../models';

    /**
     * OpenCC constructor.
     * @param string|null $dictDir
     */
    private function __construct(string $dictDir = null)
    {
        if (!ini_get('ffi.enable')) {
            throw new RuntimeException("请设置php.ini中的ffi.enable参数");
        }
        $this->ffi = $this->makeFFI();
        if ($dictDir && file_exists($dictDir)) {
            $this->dictDir = $dictDir;
        }
        $this->ffi->init(realpath($this->dictDir), "", 1024*1024*16, 0, 0);
    }

    /**
     *
     */
    public function __destruct()
    {
        // TODO: Implement __destruct() method.
        if ($this->ffi) {
            $this->ffi->deinit();
        }
    }

    /**
     * 简体转繁体
     *
     * @param string $str
     * @return array ['words' => '', 'tags' => '', 'weight' = > '']
     */
    public function parse(string $str): array
    {
        $segments = ['words' => '', 'tags' => '', 'weight' => ''];
        if (empty($str)) {
            return $segments;
        }
        $segVal = $this->ffi->seg($str);
        if ($segVal < 1) {
            return $segments;
        }
        $resultStr = $this->ffi->getResult();
        $this->ffi->freeResult();
        $words = explode(" ", $resultStr);
        foreach ($words as $word) {
            $explode = explode('_', $word);
            $segments['words'] .= $explode[0] . ' ';
            $segments['tags'] .= ($explode[1] ?? '') . ' ';
        }
        return $segments;
    }

    /**
     * @param string|null $dictPath
     * @return static
     */
    public static function new(string $dictPath = null): ThuLAC
    {
        if (self::$cc == null) {
            self::$cc = new static($dictPath);
        }
        return self::$cc;
    }

    /**
     *
     */
    private function __clone()
    {

    }

    /**
     * @return FFI
     */
    private function makeFFI(): FFI
    {
        return FFI::cdef(
            file_get_contents(__DIR__ . '/../lib/libthulac.h'),
            $this->defaultLibraryPath()
        );
    }

    /**
     * @return string
     */
    private function defaultLibraryPath(): string
    {
        if (PHP_INT_SIZE !== 8) {
            throw new RuntimeException('不支持32位系统，请自行编译lib文件');
        }
        $suffix = PHP_SHLIB_SUFFIX;
        if (PHP_OS == 'Darwin') {
            $suffix = 'dylib';
        }
        $filepath = __DIR__ . '/../lib/libthulac.' . $suffix;
        if (file_exists($filepath)) {
            return realpath($filepath);
        }
        throw new RuntimeException('不支持的系统，请自行编译lib文件');
    }
}
