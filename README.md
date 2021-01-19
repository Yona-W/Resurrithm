# Resurrithm

WIP project to port Seaurchin to Linux / Vulkan and fix some of its issues to make it:
- closer to the arcade game
- more compatible with OpeNITHM
- playable by people who can't read japanese

The original Readme
某スライドして手を跳ね上げる音ゲーシミュレーター

## ダウンロード
[Releases](https://github.com/kb10uy/Seaurchin/releases)

## ドキュメント
* [公式Wiki](https://seaurchin.kb10uy.org) - メインストリームドキュメント
* [非公式Wiki](https://www65.atwiki.jp/seaurchin-fan/) - 創作譜面などはこちらに

## 使用ライブラリとか
* [DXライブラリ](http://dxlib.o.oo7.jp/)
* [AngelScript](http://www.angelcode.com/angelscript/)
* [Boost](https://www.boost.org/)
* [FreeType](https://www.freetype.org/)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [Wacom feel™ Multi-Touch SDK](http://wdnet.jp/library/feelmulti-touch)
* [spdlog](https://github.com/gabime/spdlog)
* [tinytoml](https://github.com/mayah/tinytoml)
* [glm](https://glm.g-truc.net/0.9.9/index.html)

## Special Thanks (敬称略)
* Amenoshita429
    - [Twitter @Amenoshita429](https://twitter.com/amenoshita_429)
    - [GitHub](https://github.com/amenoshita-429)
    - **Amenoshitaスキンの開発**
    - **スキンメモリリークの解消(0.50.0)**
    - **不可視中継点問題の解決(0.44.0)**
    - **スキン機能の拡充、検証、実用**
    - **様々な(クリティカルな)バグ報告**
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
> 
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
> 
> ### Bass Audio Library
> TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, BASS IS PROVIDED
> "AS IS", WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
> INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY
> AND/OR FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS SHALL NOT BE HELD
> LIABLE FOR ANY DAMAGE THAT MAY RESULT FROM THE USE OF BASS. YOU USE
> BASS ENTIRELY AT YOUR OWN RISK.
> 
> Usage of BASS indicates that you agree to the above conditions.
> 
> All trademarks and other registered names contained in the BASS
> package are the property of their respective owners.
> 
> MP3 technology is patented, and Thomson license the use of their and
> Fraunhofer's patents. The inclusion of an MP3 decoder (eg. BASS) in a
> commercial product requires an MP3 patent licence. Contact Thomson for
> details:
> 
> 	www.mp3licensing.com
> 
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
> 
> ### RxCpp
> Reactive-Extensions/RxCpp is licensed under the
> Apache License 2.0
> 
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
