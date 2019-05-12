# Seaurchin
某スライドして手を跳ね上げる音ゲーシミュレーター

このプロジェクトは [kb10uy](https://github.com/kb10uy)氏が開発されていた音ゲーシミュレーター「Seaurchin」に
[Amenoshita-429](https://github.com/Amenoshita-429)が独自に改良、修正、拡張を加えたものになります。
現段階ではこのプロジェクトにおける開発は**Seaurchinの正式な後継バージョンの開発ではありません**のでご注意ください。

このAmenoshita-429版Seaurchinはkb10uy版Seaurchinとは一部仕様が異なる可能性があります。
また、予告なく仕様、挙動が変更される可能性が大いにあります。予めご了承ください。

Amenoshita-429版Seaurchinはkb10uy版Seaurchinに対し若干の軽量化と動作の安定を図るために比較的大規模な変更を行っています。
そのためkb10uy版で正常に動作していた機能がAmenoshita-429版ではむしろ正常に機能しない可能性があります。

## ダウンロード
当面の間は[こちら](https://github.com/kb10uy/Seaurchin/releases)からkb10uy版Seaurchinがダウンロードが可能だと思われます。

Amenoshita-429版Seaurchinは将来的に[こちら](https://github.com/Amenoshita-429/Seaurchin/releases)からダウンロードが可能になる可能性があります。

## ドキュメント
* [公式Wiki](https://seaurchin.kb10uy.org) - kb10uy氏により提供されているメインストリームドキュメント。Amenoshita-429版Seaurchinはこのドキュメントに**準拠しない可能性があります**。
* [非公式Wiki](https://www65.atwiki.jp/seaurchin-fan/) - 有志により運営されている非公式Wiki。創作譜面などの情報はこちらにお願いします。

## 使用ライブラリとか
* [DXライブラリ](http://dxlib.o.oo7.jp/)
* [AngelScript](http://www.angelcode.com/angelscript/)
* [FreeType](https://www.freetype.org/)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [spdlog](https://github.com/gabime/spdlog)
* [tinytoml](https://github.com/mayah/tinytoml)
* [glm](https://glm.g-truc.net/0.9.9/index.html)

## Special Thanks (敬称略)
* paralleltree
    - [Twitter @paralleltree](https://twitter.com/paralleltree)
    - [GitHub](https://github.com/paralleltree)
    - **Chedの開発**
* mizucoffee
    - [Twitter @kawakawaritsuki](https://twitter.com/kawakawaritsuki)
    - [GitHub](https://github.com/mizucoffee)
    - **Seaurchin Bootstrapの整備**
* t4nishi
    - [Twitter @t4nishi](https://twitter.com/t4nishi)
    - [GitHub](https://github.com/TinyTany)
    - **NotesEditorForD, M4pleの開発**
* Discord「Seaurchin 創作譜面勢」のみなさん
    - **様々なバグ報告・要望**

## ライセンス
本体はMIT License
その他ライブラリは以下参照

## 使用ライブラリ ライセンス
> ### libjpeg
> libjpeg　Copyright (C) 1991-2013, Thomas G. Lane, Guido Vollbeding.
> 
> ###libpng
> libpng　Copyright (C) 2004, 2006-2012 Glenn Randers-Pehrson.
> zlib　Copyright (C) 1995-2012 Jean-loup Gailly and Mark Adler.
> 
> ### libtiff
> libtiff　Copyright (c) 1988-1997 Sam Leffler
> libtiff　Copyright (c) 1991-1997 Silicon Graphics, Inc.

> ### ogg/vorbis
> Copyright (C) 2002-2009 Xiph.org Foundation
> 
> Redistribution and use in source and binary forms, with or without
> modification, are permitted provided that the following conditions
> are met:
> 
> - Redistributions of source code must retain the above copyright
> notice, this list of conditions and the following disclaimer.
> 
> - Redistributions in binary form must reproduce the above copyright
> notice, this list of conditions and the following disclaimer in the
> documentation and/or other materials provided with the distribution.
> 
> - Neither the name of the Xiph.org Foundation nor the names of its
> contributors may be used to endorse or promote products derived from
> this software without specific prior written permission.
> 
> THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
> ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
> LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
> A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE FOUNDATION
> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
> SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
> LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
> DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
> THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
> (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
> OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

> ### Mersenne Twister
> Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
> All rights reserved.
> 
> Redistribution and use in source and binary forms, with or without
> modification, are permitted provided that the following conditions
> are met:
> 
> 1. Redistributions of source code must retain the above copyright
> notice, this list of conditions and the following disclaimer.
> 
> 2. Redistributions in binary form must reproduce the above copyright
> notice, this list of conditions and the following disclaimer in the
> documentation and/or other materials provided with the distribution.
> 
> 3. The name of the author may not be used to endorse or promote products
> derived from this software without specific prior written permission.
> 
> THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
> IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
> OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
> IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
> INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
> NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
> DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
> THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
> (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
> THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

> ### AngelCode Scripting Library
> 
> Copyright © 2003-2016 Andreas Jönsson
> 
> This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
> 
> Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
> 
> The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
> 
> Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
> 
> This notice may not be removed or altered from any source distribution.

> ### tinytoml
> Copyright (c) 2014, MAYAH
> All rights reserved.
> 
> Redistribution and use in source and binary forms, with or without
> modification, are permitted provided that the following conditions are met:
> 
> * Redistributions of source code must retain the above copyright notice, this
>   list of conditions and the following disclaimer.
> 
> * Redistributions in binary form must reproduce the above copyright notice,
>   this list of conditions and the following disclaimer in the documentation
>   and/or other materials provided with the distribution.
> 
> THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
> AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
> IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
> DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
> FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
> DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
> SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
> CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
> OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
> OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

> ### RxCpp
> Reactive-Extensions/RxCpp is licensed under the
> Apache License 2.0

> ### GLM
> ================================================================================
> OpenGL Mathematics (GLM)
> --------------------------------------------------------------------------------
> GLM is licensed under The Happy Bunny License and MIT License
> 
> ================================================================================
> The Happy Bunny License (Modified MIT License)
> --------------------------------------------------------------------------------
> Copyright (c) 2005 - 2014 G-Truc Creation
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in
> all copies or substantial portions of the Software.
> 
> Restrictions:
>  By making use of the Software for military purposes, you choose to make a
>  Bunny unhappy.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
> THE SOFTWARE.

> ### fmt
> Copyright (c) 2012 - 2016, Victor Zverovich
> 
> All rights reserved.
> 
> Redistribution and use in source and binary forms, with or without modification,
> are permitted provided that the following conditions are met:
> 
> Redistributions of source code must retain the above copyright notice,
> this list of conditions and the following disclaimer.
> 
> Redistributions in binary form must reproduce the above copyright notice,
> this list of conditions and the following disclaimer in the documentation
> and/or other materials provided with the distribution.
> 
> THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
> AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
> IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
> DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
> ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
> (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
> LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
> THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
> NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
> ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
