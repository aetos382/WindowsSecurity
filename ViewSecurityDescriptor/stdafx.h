// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#include <tchar.h>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include <filesystem>
#include <stdexcept>

// https://github.com/okdshin/unique_resource
#include "../unique_resource.hpp"

#include <Windows.h>
#include <AclAPI.h>
