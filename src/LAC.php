<?php
namespace FastFFI\LAC;

use FFI;
use RuntimeException;

class LAC
{
    /**
     * @var ?LAC
     */
    private static ?LAC $cc = null;

    /**
     * @var FFI
     */
    protected FFI $ffi;

    /**
     * @var string
     */
    protected string $dictDir = __DIR__ . '/../model/lac_model';

    /**
     * @var FFI\CData
     */
    private $lac;

    /**
     * OpenCC constructor.
     * @param string|null $dictDir
     */
    private function __construct(string $dictDir = null)
    {
        if (ini_get('ffi.enable') == false) {
            throw new RuntimeException("请设置php.ini中的ffi.enable参数");
        }
        $this->ffi = $this->makeFFI();
        if ($dictDir && file_exists($dictDir)) {
            $this->dictDir = $dictDir;
        }
        $this->lac = $this->ffi->new_lac($this->dictDir);
    }

    /**
     *
     */
    public function __destruct()
    {
        // TODO: Implement __destruct() method.
        if ($this->ffi) {
            $this->ffi->free_lac($this->lac);
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
        if (empty($str)) {
            return ['words' => '', 'tags' => '', 'weight' => ''];
        }
        $struct = $this->ffi->parse($str, $this->lac);
        return $this->convert($struct);
    }

    /**
     * @param string|null $dictPath
     * @return static
     */
    public static function new(string $dictPath = null): LAC
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
     * @param FFI\CData $CData
     * @return array
     */
    private function convert(FFI\CData $CData): array
    {
        $result = [
            'words' => FFI::string($CData->words),
            'tags'  => FFI::string($CData->tags),
            'weight' => FFI::string($CData->weight),
        ];
        $this->ffi->free_result($CData);
         return $result;
    }

    /**
     * @return FFI
     */
    private function makeFFI(): FFI
    {
        return FFI::cdef(
            file_get_contents(__DIR__ . '/../lib/liblacffi.h'),
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
        $filepath = __DIR__ . '/../lib/liblacffi.' . $suffix;
        if (file_exists($filepath)) {
            return realpath($filepath);
        }
        throw new RuntimeException('不支持的系统，请自行编译lib文件');
    }
}
