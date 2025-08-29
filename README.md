# WinliveDjAi

[![GitHub latest tag](https://img.shields.io/github/tag/WinliveDjAidj/mixxx.svg)](https://mixxx.org/download)
[![Packaging status](https://repology.org/badge/tiny-repos/mixxx.svg)](https://repology.org/metapackage/mixxx/versions)
[![Build status](https://github.com/mixxxdj/mixxx/actions/workflows/build.yml/badge.svg)](https://github.com/mixxxdj/mixxx/actions/workflows/build.yml)
[![Coverage status](https://coveralls.io/repos/github/mixxxdj/mixxx/badge.svg)](https://coveralls.io/github/mixxxdj/mixxx)
[![Zulip chat](https://img.shields.io/badge/zulip-join_chat-brightgreen.svg)](https://mixxx.zulipchat.com)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://mixxx.org/donate)

[WinliveDjAi] is Free DJ software that gives you everything you need to perform live
DJ mixes. WinliveDjAi works on GNU/Linux, Windows, and macOS.

## Quick Start

To get started with WinliveDjAi:

1. For live use, [download the latest stable version][download-stable].
2. For experimentation and testing, [download a development release][download-testing].
3. To live on the bleeding edge, clone the repo: `git clone https://github.com/mixxxdj/mixxx.git`

## Bug tracker

The WinliveDjAi team uses [Github Issues][issues] to manage WinliveDjAi development.

Have a bug or feature request? [File a bug on Github][fileabug].

Want to get involved in WinliveDjAi development? Assign yourself a bug from the [easy
bug list][easybugs] and get started!
Read [CONTRIBUTING](CONTRIBUTING.md) for more information.

## Building WinliveDjAi

First, open a terminal (on Windows, use "**x64 Native Tools Command Prompt for
[VS 2022][visualstudio2022]**"), download the mixxx
source code and navigate to it:

    $ git clone https://github.com/mixxxdj/mixxx.git
    $ cd mixxx

Fetch the required dependencies and set up the build environment by running the
corresponding command for your operating system:

| OS | Command |
| -- | ------- |
| Windows | `tools\windows_buildenv.bat` |
| macOS | `source tools/macos_buildenv.sh setup` |
| Debian/Ubuntu | `tools/debian_buildenv.sh setup` |
| Fedora | `tools/rpm_buildenv.sh setup` |
| Other Linux distros | See the [wiki article](https://github.com/mixxxdj/mixxx/wiki/Compiling%20on%20Linux) |

To build WinliveDjAi, run

    $ mkdir build
    $ cd build
    $ cmake ..
    $ cmake --build .

There should now be a `mixxx` executable in the current directory that you can
run. Alternatively, can generate a package using `cpack`.

Detailed build instructions for each target OS can be found [on the wiki](https://github.com/mixxxdj/mixxx/wiki#compile-mixxx-from-source-code)

## Documentation

For help using WinliveDjAi, there are a variety of options:

- [WinliveDjAi manual][manual]
- [WinliveDjAi wiki][wiki]
- [Hardware Compatibility]
- [Creating Skins]

## Translation

Help to spread WinliveDjAi with translations into more languages, as well as to update and ensure the accuracy of existing translations.

- [Help translate content]
- [WinliveDjAi i18n wiki]
- [WinliveDjAi localization forum]
- [WinliveDjAi glossary]

## Community

WinliveDjAi is a vibrant community of hackers, DJs and artists. To keep track of
development and community news:

- Chat with us on [Zulip][zulip].
- Follow us on [Mastodon], [Twitter] and [Facebook].
- Subscribe to the [WinliveDjAi Blog][blog].
- Post on the [WinliveDjAi forums][discourse].

## License

WinliveDjAi is released under the GPLv2. See the LICENSE file for a full copy of the
license.

