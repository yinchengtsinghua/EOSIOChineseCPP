
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

//根据3条款BSD许可证分发。见随附
//文件许可证或https://github.com/cliutils/cli11了解详细信息。

//此文件是使用cli11/scripts中的makesingleheader.py生成的
//自：v1.1.0-17-GB88F1F2

//它在一个文件中有完整的CLI库。

#include <sys/types.h>
#include <iostream>
#include <string>
#include <deque>
#include <memory>
#include <algorithm>
#include <tuple>
#include <iomanip>
#include <type_traits>
#include <functional>
#include <numeric>
#include <fstream>
#include <vector>
#include <locale>
#include <set>
#include <stdexcept>
#include <exception>
#include <sstream>
#include <sys/stat.h>
#include <utility>

//来自cli/error.hpp

namespace CLI {

///这些代码是CLI中每个错误的一部分。它们可以使用e.exit_代码或作为快捷方式从e中获取，
///int来自e.get_error_code（）的值。
enum class ExitCodes {
    Success = 0,
    IncorrectConstruction = 100,
    BadNameString,
    OptionAlreadyAdded,
    File,
    Conversion,
    Validation,
    Required,
    Requires,
    Excludes,
    Extras,
    ExtrasINI,
    Invalid,
    Horrible,
    OptionNotFound,
    BaseClass = 255
};

//错误定义

///@defgroup错误\u组错误
///@cli11抛出的简短错误
///
///这些是可以引发的错误。其中一些错误，如cli:：success，并不是真正的错误。
///@

///所有错误都源于此错误
struct Error : public std::runtime_error {
    int exit_code;
    bool print_help;
    int get_exit_code() const { return exit_code; }
    Error(std::string parent, std::string name, ExitCodes exit_code = ExitCodes::BaseClass, bool print_help = true)
        : runtime_error(parent + ": " + name), exit_code(static_cast<int>(exit_code)), print_help(print_help) {}
    Error(std::string parent,
          std::string name,
          int exit_code = static_cast<int>(ExitCodes::BaseClass),
          bool print_help = true)
        : runtime_error(parent + ": " + name), exit_code(exit_code), print_help(print_help) {}
};

///构造错误（不在分析中）
struct ConstructionError : public Error {
//使用错误：：错误构造函数似乎不适用于GCC4.7。
    ConstructionError(std::string parent,
                      std::string name,
                      ExitCodes exit_code = ExitCodes::BaseClass,
                      bool print_help = true)
        : Error(parent, name, exit_code, print_help) {}
};

///当选项设置为冲突值时引发（例如，非向量和多参数）
struct IncorrectConstruction : public ConstructionError {
    IncorrectConstruction(std::string name)
        : ConstructionError("IncorrectConstruction", name, ExitCodes::IncorrectConstruction) {}
};

///在构造坏名称时引发
struct BadNameString : public ConstructionError {
    BadNameString(std::string name) : ConstructionError("BadNameString", name, ExitCodes::BadNameString) {}
};

///当选项已存在时引发
struct OptionAlreadyAdded : public ConstructionError {
    OptionAlreadyAdded(std::string name)
        : ConstructionError("OptionAlreadyAdded", name, ExitCodes::OptionAlreadyAdded) {}
};

//解析错误

///分析中可能出错的任何内容
struct ParseError : public Error {
    ParseError(std::string parent, std::string name, ExitCodes exit_code = ExitCodes::BaseClass, bool print_help = true)
        : Error(parent, name, exit_code, print_help) {}
};

//不是真正的“错误”

///This is a successful complete on parsing，should to exit/这是分析成功完成，应该退出。
struct Success : public ParseError {
    Success() : ParseError("Success", "Successfully completed, should be caught and quit", ExitCodes::Success, false) {}
};

///-h或--命令行帮助
struct CallForHelp : public ParseError {
    CallForHelp()
        : ParseError("CallForHelp", "This should be caught in your main function, see examples", ExitCodes::Success) {}
};

///分析ini文件时引发，但该文件丢失
struct FileError : public ParseError {
    FileError(std::string name) : ParseError("FileError", name, ExitCodes::File) {}
};

///在转换回调失败时引发，例如当int未能与字符串共存时
struct ConversionError : public ParseError {
    ConversionError(std::string name) : ParseError("ConversionError", name, ExitCodes::Conversion) {}
};

///在结果验证失败时引发
struct ValidationError : public ParseError {
    ValidationError(std::string name) : ParseError("ValidationError", name, ExitCodes::Validation) {}
};

///缺少必需选项时引发
struct RequiredError : public ParseError {
    RequiredError(std::string name) : ParseError("RequiredError", name, ExitCodes::Required) {}
};

///Requires选项丢失时引发
struct RequiresError : public ParseError {
    RequiresError(std::string name, std::string subname)
        : ParseError("RequiresError", name + " requires " + subname, ExitCodes::Requires) {}
};

///exludes选项存在时引发
struct ExcludesError : public ParseError {
    ExcludesError(std::string name, std::string subname)
        : ParseError("ExcludesError", name + " excludes " + subname, ExitCodes::Excludes) {}
};

///在找到太多位置或选项时引发
struct ExtrasError : public ParseError {
    ExtrasError(std::string name) : ParseError("ExtrasError", name, ExitCodes::Extras) {}
};

///ini文件中找到额外值时引发
struct ExtrasINIError : public ParseError {
    ExtrasINIError(std::string name) : ParseError("ExtrasINIError", name, ExitCodes::ExtrasINI) {}
};

///分析前验证失败时引发
struct InvalidError : public ParseError {
    InvalidError(std::string name) : ParseError("InvalidError", name, ExitCodes::Invalid) {}
};

///这只是一个安全检查，用于验证选择和分析是否匹配。
struct HorribleError : public ParseError {
    HorribleError(std::string name)
        : ParseError("HorribleError", "(You should never see this error) " + name, ExitCodes::Horrible) {}
};

//解析后

///在计算不存在的选项时引发
struct OptionNotFound : public Error {
    OptionNotFound(std::string name) : Error("OptionNotFound", name, ExitCodes::OptionNotFound) {}
};

///@

} //命名空间CLI

//来自cli/typetools.hpp

namespace CLI {

//类型工具

//从C++ 14复制
#if __cplusplus < 201402L
template <bool B, class T = void> using enable_if_t = typename std::enable_if<B, T>::type;
#else
//如果编译器支持C++ 14，则可以使用该定义而不是
using std::enable_if_t;
#endif

template <typename T> struct is_vector { static const bool value = false; };

template <class T, class A> struct is_vector<std::vector<T, A>> { static bool const value = true; };

template <typename T> struct is_bool { static const bool value = false; };

template <> struct is_bool<bool> { static bool const value = true; };

namespace detail {
//一般基于https://rmf.io/cxx11/most-static-if
///simple空作用域类
enum class enabler {};

///ENABLEIF中要使用的实例
constexpr enabler dummy = {};

//类型名称打印

///将基于
///http://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
///但这样比较干净，在这种情况下效果更好

template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "INT";
}

template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "UINT";
}

template <typename T, enable_if_t<std::is_floating_point<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "FLOAT";
}

///this one不应使用，因为vector类型打印内部类型
template <typename T, enable_if_t<is_vector<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "VECTOR";
}

template <typename T,
          enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value && !is_vector<T>::value,
                      detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "TEXT";
}

//词汇投射

///整数/枚举
template <typename T, enable_if_t<std::is_integral<T>::value
    || std::is_enum<T>::value
    , detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    try {
        output = static_cast<T>(std::stoll(input));
        return true;
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        return false;
    }
}

//漂浮物
template <typename T, enable_if_t<std::is_floating_point<T>::value, detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    try {
        output = static_cast<T>(std::stold(input));
        return true;
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        return false;
    }
}

///string及类似
template <
    typename T,
    enable_if_t<!std::is_floating_point<T>::value
                && !std::is_integral<T>::value
                && !std::is_enum<T>::value, detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    output = input;
    return true;
}

} //命名空间详细信息
} //命名空间CLI

//来自cli/stringtools.hpp

namespace CLI {
namespace detail {

//基于http://stackoverflow.com/questions/236129/split-a-string-in-c
///由熟食店拆分字符串
inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
//检查EMTPY字符串是否一致
    if(s == "")
        elems.emplace_back("");
    else {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while(std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
    }
    return elems;
}

///simple函数连接字符串
template <typename T> std::string join(const T &v, std::string delim = ",") {
    std::ostringstream s;
    size_t start = 0;
    for(const auto &i : v) {
        if(start++ > 0)
            s << delim;
        s << i;
    }
    return s.str();
}

///按相反顺序联接字符串
template <typename T> std::string rjoin(const T &v, std::string delim = ",") {
    std::ostringstream s;
    for(size_t start = 0; start < v.size(); start++) {
        if(start > 0)
            s << delim;
        s << v[v.size() - start - 1];
    }
    return s.str();
}

//大致基于http://stackoverflow.com/questions/25829143/c-trim-whitespace-from-a-string

///trim字符串左边的空白
inline std::string &ltrim(std::string &str) {
    auto it = std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace<char>(ch, std::locale()); });
    str.erase(str.begin(), it);
    return str;
}

///trim字符串左侧的任何内容
inline std::string &ltrim(std::string &str, const std::string &filter) {
    auto it = std::find_if(str.begin(), str.end(), [&filter](char ch) { return filter.find(ch) == std::string::npos; });
    str.erase(str.begin(), it);
    return str;
}

///trim字符串右侧的空白
inline std::string &rtrim(std::string &str) {
    auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale()); });
    str.erase(it.base(), str.end());
    return str;
}

///trim字符串右侧的任何内容
inline std::string &rtrim(std::string &str, const std::string &filter) {
    auto it =
        std::find_if(str.rbegin(), str.rend(), [&filter](char ch) { return filter.find(ch) == std::string::npos; });
    str.erase(it.base(), str.end());
    return str;
}

///trim字符串中的空白
inline std::string &trim(std::string &str) { return ltrim(rtrim(str)); }

///trim字符串中的任何内容
inline std::string &trim(std::string &str, const std::string filter) { return ltrim(rtrim(str, filter), filter); }

///复制字符串，然后修剪它
inline std::string trim_copy(const std::string &str) {
    std::string s = str;
    return trim(s);
}

///make a copy of the string and then trim it，any filter string can be used（any char in string is filtered）
inline std::string trim_copy(const std::string &str, const std::string &filter) {
    std::string s = str;
    return trim(s, filter);
}
///打印由两部分组成的“帮助”字符串
inline void format_help(std::stringstream &out, std::string name, std::string description, size_t wid) {
    name = "  " + name;
    out << std::setw(static_cast<int>(wid)) << std::left << name;
    if(description != "") {
        if(name.length() >= wid)
            out << std::endl << std::setw(static_cast<int>(wid)) << "";
        out << description;
    }
    out << std::endl;
}

///验证选项的第一个字符
template <typename T> bool valid_first_char(T c) { return std::isalpha(c, std::locale()) || c == '_'; }

///验证选项的下列字符
template <typename T> bool valid_later_char(T c) {
    return std::isalnum(c, std::locale()) || c == '_' || c == '.' || c == '-';
}

///验证选项名
inline bool valid_name_string(const std::string &str) {
    if(str.empty() || !valid_first_char(str[0]))
        return false;
    for(auto c : str.substr(1))
        if(!valid_later_char(c))
            return false;
    return true;
}

///返回字符串的小写版本
inline std::string to_lower(std::string str) {
    std::transform(std::begin(str), std::end(str), std::begin(str), [](const std::string::value_type &x) {
        return std::tolower(x, std::locale());
    });
    return str;
}

///将字符串“one two”“three”拆分为“one two”“three”
inline std::vector<std::string> split_up(std::string str) {

    std::vector<char> delims = {'\'', '\"'};
    auto find_ws = [](char ch) { return std::isspace<char>(ch, std::locale()); };
    trim(str);

    std::vector<std::string> output;

    while(!str.empty()) {
        if(str[0] == '\'') {
            auto end = str.find('\'', 1);
            if(end != std::string::npos) {
                output.push_back(str.substr(1, end - 1));
                str = str.substr(end + 1);
            } else {
                output.push_back(str.substr(1));
                str = "";
            }
        } else if(str[0] == '\"') {
            auto end = str.find('\"', 1);
            if(end != std::string::npos) {
                output.push_back(str.substr(1, end - 1));
                str = str.substr(end + 1);
            } else {
                output.push_back(str.substr(1));
                str = "";
            }

        } else {
            auto it = std::find_if(std::begin(str), std::end(str), find_ws);
            if(it != std::end(str)) {
                std::string value = std::string(str.begin(), it);
                output.push_back(value);
                str = std::string(it, str.end());
            } else {
                output.push_back(str);
                str = "";
            }
        }
        trim(str);
    }

    return output;
}

} //命名空间详细信息
} //命名空间CLI

//来自cli/split.hpp

namespace CLI {
namespace detail {

//如果不是短选项，则返回false。否则，设置opt name和rest并返回true
inline bool split_short(const std::string &current, std::string &name, std::string &rest) {
    if(current.size() > 1 && current[0] == '-' && valid_first_char(current[1])) {
        name = current.substr(1, 1);
        rest = current.substr(2);
        return true;
    } else
        return false;
}

//如果不是长选项，则返回false。否则，将opt name和other side设置为=并返回true
inline bool split_long(const std::string &current, std::string &name, std::string &value) {
    if(current.size() > 2 && current.substr(0, 2) == "--" && valid_first_char(current[2])) {
        auto loc = current.find("=");
        if(loc != std::string::npos) {
            name = current.substr(2, loc - 2);
            value = current.substr(loc + 1);
        } else {
            name = current.substr(2);
            value = "";
        }
        return true;
    } else
        return false;
}

//将字符串拆分为多个长名称和短名称
inline std::vector<std::string> split_names(std::string current) {
    std::vector<std::string> output;
    size_t val;
    while((val = current.find(",")) != std::string::npos) {
        output.push_back(current.substr(0, val));
        current = current.substr(val + 1);
    }
    output.push_back(current);
    return output;
}

///获取短名称、长名称之一和单个名称的向量
inline std::tuple<std::vector<std::string>, std::vector<std::string>, std::string>
get_names(const std::vector<std::string> &input) {

    std::vector<std::string> short_names;
    std::vector<std::string> long_names;
    std::string pos_name;

    for(std::string name : input) {
        if(name.length() == 0)
            continue;
        else if(name.length() > 1 && name[0] == '-' && name[1] != '-') {
            if(name.length() == 2 && valid_first_char(name[1]))
                short_names.emplace_back(1, name[1]);
            else
                throw BadNameString("Invalid one char name: " + name);
        } else if(name.length() > 2 && name.substr(0, 2) == "--") {
            name = name.substr(2);
            if(valid_name_string(name))
                long_names.push_back(name);
            else
                throw BadNameString("Bad long name: " + name);
        } else if(name == "-" || name == "--") {
            throw BadNameString("Must have a name, not just dashes");
        } else {
            if(pos_name.length() > 0)
                throw BadNameString("Only one positional name allowed, remove: " + name);
            pos_name = name;
        }
    }

    return std::tuple<std::vector<std::string>, std::vector<std::string>, std::string>(
        short_names, long_names, pos_name);
}

} //命名空间详细信息
} //命名空间CLI

//来自cli/ini.hpp

namespace CLI {
namespace detail {

inline std::string inijoin(std::vector<std::string> args) {
    std::ostringstream s;
    size_t start = 0;
    for(const auto &arg : args) {
        if(start++ > 0)
            s << " ";

        auto it = std::find_if(arg.begin(), arg.end(), [](char ch) { return std::isspace<char>(ch, std::locale()); });
        if(it == arg.end())
            s << arg;
        else if(arg.find(R"(")") == std::string::npos)
            s << R"(")" << arg << R"(")";
        else
            s << R"(')" << arg << R"(')";
    }

    return s.str();
}

struct ini_ret_t {
///这是带点的全名
    std::string fullname;

///输入列表
    std::vector<std::string> inputs;

///当前父级
    size_t level = 0;

///返回父字符串或空字符串，基于级别
///
///0级，A.B.C将返回
///1级，A.B.C可以返回B
    std::string parent() const {
        std::vector<std::string> plist = detail::split(fullname, '.');
        if(plist.size() > (level + 1))
            return plist[level];
        else
            return "";
    }

//返回名称
    std::string name() const {
        std::vector<std::string> plist = detail::split(fullname, '.');
        return plist.at(plist.size() - 1);
    }
};

///内部分析函数
inline std::vector<ini_ret_t> parse_ini(std::istream &input) {
    std::string name, line;
    std::string section = "default";

    std::vector<ini_ret_t> output;

    while(getline(input, line)) {
        std::vector<std::string> items;

        detail::trim(line);
        size_t len = line.length();
        if(len > 1 && line[0] == '[' && line[len - 1] == ']') {
            section = line.substr(1, len - 2);
        } else if(len > 0 && line[0] != ';') {
            output.emplace_back();
            ini_ret_t &out = output.back();

//find=在字符串中，拆分并重新组合
            auto pos = line.find("=");
            if(pos != std::string::npos) {
                name = detail::trim_copy(line.substr(0, pos));
                std::string item = detail::trim_copy(line.substr(pos + 1));
                items = detail::split_up(item);
            } else {
                name = detail::trim_copy(line);
                items = {"ON"};
            }

            if(detail::to_lower(section) == "default")
                out.fullname = name;
            else
                out.fullname = section + "." + name;

            out.inputs.insert(std::end(out.inputs), std::begin(items), std::end(items));
        }
    }
    return output;
}

///parse一个ini文件，失败时引发一个错误（parserror:iniparserror或file error）。
inline std::vector<ini_ret_t> parse_ini(const std::string &name) {

    std::ifstream input{name};
    if(!input.good())
        throw FileError(name);

    return parse_ini(input);
}

} //命名空间详细信息
} //命名空间CLI

//来自cli/validators.hpp

namespace CLI {

///@defgroup验证器\u组验证器
///@简要介绍提供的一些验证器
///
///These are simple`bool（std:：string）` validator that are used.
///@

///检查现有文件
inline bool ExistingFile(std::string filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
    if(!exist) {
        std::cerr << "File does not exist: " << filename << std::endl;
        return false;
    } else if(is_dir) {
        std::cerr << "File is actually a directory: " << filename << std::endl;
        return false;
    } else {
        return true;
    }
}

///检查现有目录
inline bool ExistingDirectory(std::string filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
    if(!exist) {
        std::cerr << "Directory does not exist: " << filename << std::endl;
        return false;
    } else if(is_dir) {
        return true;
    } else {
        std::cerr << "Directory is actually a file: " << filename << std::endl;
        return false;
    }
}

///检查不存在的路径
inline bool NonexistentPath(std::string filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    if(!exist) {
        return true;
    } else {
        std::cerr << "Path exists: " << filename << std::endl;
        return false;
    }
}

///生成一个范围验证器函数
template <typename T> std::function<bool(std::string)> Range(T min, T max) {
    return [min, max](std::string input) {
        T val;
        detail::lexical_cast(input, val);
        return val >= min && val <= max;
    };
}

///一个值的范围是0到值
template <typename T> std::function<bool(std::string)> Range(T max) { return Range(static_cast<T>(0), max); }

///@

} //命名空间CLI

//来自cli/option.hpp

namespace CLI {

using results_t = std::vector<std::string>;
using callback_t = std::function<bool(results_t)>;

class Option;
class App;

using Option_p = std::unique_ptr<Option>;

class Option {
    friend App;

  protected:
//名称名称
///@

///a没有前导破折号的短名称列表（`-a`）
    std::vector<std::string> snames_;

///a没有前导破折号的长名称列表（`--a`）
    std::vector<std::string> lnames_;

///a位置名
    std::string pname_;

///如果给定，请检查此选项的环境
    std::string envname_;

///@
//@名称帮助
///@

///帮助字符串的说明
    std::string description_;

///a人类可读的默认值，通常仅当创建时默认值为true时设置。
    std::string defaultval_;

///a人类可读的类型值，当应用程序创建此值时设置
    std::string typeval_;

///组成员身份
    std::string group_{"Options"};

///true（如果此选项有默认值）
    bool default_{false};

///@
///@name配置
///@

///true（如果这是必需选项）
    bool required_{false};

///期望值的数目，0代表标志，1代表无限向量
    int expected_{1};

///a允许args不能接受不正确的预期值的私有设置
    bool changeable_{false};

///匹配时忽略大小写（选项，而不是值）
    bool ignore_case_{false};

///a要对分析的每个值运行的验证程序列表
    std::vector<std::function<bool(std::string)>> validators_;

///a此选项所需的选项列表
    std::set<Option *> requires_;

///a此选项排除的选项列表
    std::set<Option *> excludes_;

///@
//@别名
///@

///记住父应用程序
    App *parent_;

///options存储回调以完成所有工作
    callback_t callback_;

///@
///@name解析结果
///@

///分析结果
    results_t results_;

///回调是否已运行（ini解析需要）
    bool callback_run_{false};

///@

///making an option by hand未定义，它必须由app类生成。
    Option(std::string name,
           std::string description = "",
           std::function<bool(results_t)> callback = [](results_t) { return true; },
           bool default_ = true,
           App *parent = nullptr)
        : description_(std::move(description)), default_(default_), parent_(parent), callback_(std::move(callback)) {
        std::tie(snames_, lnames_, pname_) = detail::get_names(detail::split_names(name));
    }

  public:
//@基本名称
///@

///count传递选项的总次数
    size_t count() const { return results_.size(); }

///this类在传递选项时为true。
    operator bool() const { return count() > 0; }

///清除分析结果（主要用于测试）
    void clear() { results_.clear(); }

///@
///@name设置选项
///@

///根据需要设置选项
    Option *required() {
        if( !required_ ) {
            description_ += " (required)";
        }
        required_ = true;
        return this;
    }

///支持多个术语
    Option *mandatory() { return required(); }

///设置所需参数的数目（标志忽略此参数）
    Option *expected(int value) {
        if(value == 0)
            throw IncorrectConstruction("Cannot set 0 expected, use a flag instead");
        else if(expected_ == 0)
            throw IncorrectConstruction("Cannot make a flag take arguments!");
        else if(!changeable_)
            throw IncorrectConstruction("You can only change the expected arguments for vectors");
        expected_ = value;
        return this;
    }

///添加验证程序
    Option *check(std::function<bool(std::string)> validator) {

        validators_.push_back(validator);
        return this;
    }

///更改组成员身份
    Option *group(std::string name) {
        group_ = name;
        return this;
    }

///设置必需选项
    Option *requires(Option *opt) {
        auto tup = requires_.insert(opt);
        if(!tup.second)
            throw OptionAlreadyAdded(get_name() + " requires " + opt->get_name());
        return this;
    }

///如果需要，可以查找字符串
    template <typename T = App> Option *requires(std::string opt_name) {
        for(const Option_p &opt : dynamic_cast<T *>(parent_)->options_)
            if(opt.get() != this && opt->check_name(opt_name))
                return requires(opt.get());
        throw IncorrectConstruction("Option " + opt_name + " is not defined");
    }

///支持的任何数字，字符串和opt的任意组合
    template <typename A, typename B, typename... ARG> Option *requires(A opt, B opt1, ARG... args) {
        requires(opt);
        return requires(opt1, args...);
    }

///设置排除的选项
    Option *excludes(Option *opt) {
        auto tup = excludes_.insert(opt);
        if(!tup.second)
            throw OptionAlreadyAdded(get_name() + " excludes " + opt->get_name());
        return this;
    }

///如果需要，可以查找字符串
    template <typename T = App> Option *excludes(std::string opt_name) {
        for(const Option_p &opt : dynamic_cast<T *>(parent_)->options_)
            if(opt.get() != this && opt->check_name(opt_name))
                return excludes(opt.get());
        throw IncorrectConstruction("Option " + opt_name + " is not defined");
    }
///支持的任何数字，字符串和opt的任意组合
    template <typename A, typename B, typename... ARG> Option *excludes(A opt, B opt1, ARG... args) {
        excludes(opt);
        return excludes(opt1, args...);
    }

///如果没有给定选项，则将环境变量设置为读取
    Option *envname(std::string name) {
        envname_ = name;
        return this;
    }

//忽略案例
///
///template隐藏了这样一个事实，即我们还没有app的定义。
///不希望在此处向模板添加参数。
    template <typename T = App> Option *ignore_case(bool value = true) {
        ignore_case_ = value;
        for(const Option_p &opt : dynamic_cast<T *>(parent_)->options_)
            if(opt.get() != this && *opt == *this)
                throw OptionAlreadyAdded(opt->get_name());
        return this;
    }

///@
///@name访问器
///@

///true（如果这是必需选项）
    bool get_required() const { return required_; }

///选项需要的参数个数
    int get_expected() const { return expected_; }

///true（如果它有默认值）
    int get_default() const { return default_; }

///true，如果参数可以直接给出
    bool get_positional() const { return pname_.length() > 0; }

///true如果选项至少有一个非位置名称
    bool nonpositional() const { return (snames_.size() + lnames_.size()) > 0; }

///true，如果选项有说明
    bool has_description() const { return description_.length() > 0; }

///获取此选项的组
    const std::string &get_group() const { return group_; }

///获取描述
    const std::string &get_description() const { return description_; }

//只是这个名字
    std::string get_pname() const { return pname_; }

///@
///@name帮助工具
///@

///获取一个SEP名称列表。如果opt_only=true，则不包括位置名称。
    std::string get_name(bool opt_only = false) const {
        std::vector<std::string> name_list;
        if(!opt_only && pname_.length() > 0)
            name_list.push_back(pname_);
        for(const std::string &sname : snames_)
            name_list.push_back("-" + sname);
        for(const std::string &lname : lnames_)
            name_list.push_back("--" + lname);
        return detail::join(name_list);
    }

///位置所需的名称和任何附加项
    std::string help_positional() const {
        std::string out = pname_;
        if(get_expected() > 1)
            out = out + "(" + std::to_string(get_expected()) + "x)";
        else if(get_expected() == -1)
            out = out + "...";
        out = get_required() ? out : "[" + out + "]";
        return out;
    }

///帮助打印的前半部分、名称加默认值等
    std::string help_name() const {
        std::stringstream out;
        out << get_name(true) << help_aftername();
        return out.str();
    }

///pname带有类型信息
    std::string help_pname() const {
        std::stringstream out;
        out << get_pname() << help_aftername();
        return out.str();
    }

///This is the part after the name is printed but before the description/这是打印名称之后但在说明之前的部分。
    std::string help_aftername() const {
        std::stringstream out;

        if(get_expected() != 0) {
            if(typeval_ != "")
                out << " " << typeval_;
            if(defaultval_ != "")
                out << "=" << defaultval_;
            if(get_expected() > 1)
                out << " x " << get_expected();
            if(get_expected() == -1)
                out << " ...";
        }
        if(envname_ != "")
            out << " (env:" << envname_ << ")";
        if(!requires_.empty()) {
            out << " Requires:";
            for(const Option *opt : requires_)
                out << " " << opt->get_name();
        }
        if(!excludes_.empty()) {
            out << " Excludes:";
            for(const Option *opt : excludes_)
                out << " " << opt->get_name();
        }
        return out.str();
    }

///@
///@name解析器工具
///@

///处理回调
    void run_callback() const {
        if(!callback_(results_))
            throw ConversionError(get_name() + "=" + detail::join(results_));
        if(!validators_.empty()) {
            for(const std::string &result : results_)
                for(const std::function<bool(std::string)> &vali : validators_)
                    if(!vali(result))
                        throw ValidationError(get_name() + "=" + result);
        }
    }

///如果选项具有相同的名称，则它们相等（不计算位置）
    bool operator==(const Option &other) const {
        for(const std::string &sname : snames_)
            if(other.check_sname(sname))
                return true;
        for(const std::string &lname : lnames_)
            if(other.check_lname(lname))
                return true;
//我们需要做相反的事情，以防万一我们忽视了
        for(const std::string &sname : other.snames_)
            if(check_sname(sname))
                return true;
        for(const std::string &lname : other.lnames_)
            if(check_lname(lname))
                return true;
        return false;
    }

///检查名称。短/长需要“-”或“--”，支持位置名称
    bool check_name(std::string name) const {

        if(name.length() > 2 && name.substr(0, 2) == "--")
            return check_lname(name.substr(2));
        else if(name.length() > 1 && name.substr(0, 1) == "-")
            return check_sname(name.substr(1));
        else {
            std::string local_pname = pname_;
            if(ignore_case_) {
                local_pname = detail::to_lower(local_pname);
                name = detail::to_lower(name);
            }
            return name == local_pname;
        }
    }

///要求从字符串中删除“-”
    bool check_sname(std::string name) const {
        if(ignore_case_) {
            name = detail::to_lower(name);
            return std::find_if(std::begin(snames_), std::end(snames_), [&name](std::string local_sname) {
                       return detail::to_lower(local_sname) == name;
                   }) != std::end(snames_);
        } else
            return std::find(std::begin(snames_), std::end(snames_), name) != std::end(snames_);
    }

///要求从字符串中删除“--”
    bool check_lname(std::string name) const {
        if(ignore_case_) {
            name = detail::to_lower(name);
            return std::find_if(std::begin(lnames_), std::end(lnames_), [&name](std::string local_sname) {
                       return detail::to_lower(local_sname) == name;
                   }) != std::end(lnames_);
        } else
            return std::find(std::begin(lnames_), std::end(lnames_), name) != std::end(lnames_);
    }

///将结果置于R位置
    void add_result(std::string s) {
        results_.push_back(s);
        callback_run_ = false;
    }

///获取结果的副本
    std::vector<std::string> results() const { return results_; }

///查看是否已运行回调
    bool get_callback_run() const { return callback_run_; }

///@
///@name自定义选项
///@

///set自定义选项，typestring，应为，且可更改。
    void set_custom_option(std::string typeval, int expected = 1, bool changeable = false) {
        typeval_ = typeval;
        expected_ = expected;
        changeable_ = changeable;
    }

///设置默认值字符串表示形式
    void set_default_val(std::string val) { defaultval_ = val; }


///设置此选项上显示的类型名
    void set_type_name(std::string val) {typeval_ = val;}

///@

  protected:
///@name应用程序帮助程序
///@
///can打印位置名称详细选项（如果为true）
    bool _has_help_positional() const {
        return get_positional() && (has_description() || !requires_.empty() || !excludes_.empty());
    }
///@
};

} //命名空间CLI

//来自cli/app.hpp

namespace CLI {

namespace detail {
enum class Classifer { NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND };
struct AppFriend;
} //命名空间详细信息

class App;

using App_p = std::unique_ptr<App>;

///创建命令行程序，很少有默认值。
/*若要使用，请使用“argc”、“argv”和帮助说明创建新的“program（）”实例。模板化
*添加选项方法使准备选项变得容易。在开始之前，请记住调用“.start”
*程序，以便评估选项，帮助选项不会意外运行程序。*/

class App final {
    friend Option;
    friend detail::AppFriend;

  protected:
//此库遵循以下划线结尾的成员名称的Google样式指南。

//@命名基础
///@

///subcommand name或程序名（来自分析器）
    std::string name_{"program"};

///当前程序/子命令的说明
    std::string description_;

///如果为true，则允许额外的参数（即不要抛出错误）。
    bool allow_extras_{false};

///如果为真，则在未识别的选项上立即返回（表示允许附加项）
    bool prefix_command_{false};
    
///这是一个在完成时运行的函数。非常适合子命令。可以扔。
    std::function<void()> callback_;

///@
//@名称选项
///@

///本地存储的选项列表
    std::vector<Option_p> options_;

///a指向帮助标志的指针（如果有）
    Option *help_ptr_{nullptr};

///@
//名称解析
///@

    using missing_t = std::vector<std::pair<detail::Classifer, std::string>>;

///对分类器，缺少选项的字符串。（从parse返回时会删除额外的细节）
///
///这比只存储字符串列表和重新分析更快、更干净。这可能包含--分隔符。
    missing_t missing_;

///这是指向具有原始分析顺序的选项的指针列表。
    std::vector<Option *> parse_order_;

///@
///@name子命令
///@

///存储子命令列表
    std::vector<App_p> subcommands_;

///if true，则程序名不区分大小写。
    bool ignore_case_{false};

///allow子命令fallthrough，以便父命令可以在子命令之后收集命令。
    bool fallthrough_{false};

///a指向父级的指针（如果这是子命令）
    App *parent_{nullptr};

///true（如果已分析此命令/子命令）
    bool parsed_{false};

///-1表示1或更多，0表示不需要，表示需要的确切数字
    int require_subcommand_ = 0;

///@
///NeX-CONFIG
///@

///连接的配置文件的名称
    std::string config_name_;

///true如果需要ini（如果不存在则抛出），如果为false，则继续。
    bool config_required_{false};

///指向配置选项的指针
    Option *config_ptr_{nullptr};

///@

///special private constructor for子命令
    App(std::string description_, bool help, detail::enabler) : description_(std::move(description_)) {

        if(help)
            help_ptr_ = add_flag("-h,--help", "Print this help message and exit");
    }

  public:
//@基本名称
///@

///创建新程序。传入与main（）相同的参数以及帮助字符串。
    App(std::string description_ = "", bool help = true) : App(description_, help, detail::dummy) {}

///为分析结束设置回调。
///
//由于C++ 11中的错误，
///不可能在STD:：函数中超载（固定在C++ 14中）
///并在新编译器上备份到C++ 11。使用引用捕获
///如果需要，获取指向应用程序的指针。
    App *set_callback(std::function<void()> callback) {
        callback_ = callback;
        return this;
    }

///remove命令行上多余的文件时出错。
    App *allow_extras(bool allow = true) {
        allow_extras_ = allow;
        return this;
    }

///不要在第一个未识别的选项之后分析任何内容并返回
    App *prefix_command(bool allow = true) {
        prefix_command_ = allow;
        return this;
    }
    
//忽略案例。子命令继承值。
    App *ignore_case(bool value = true) {
        ignore_case_ = value;
        if(parent_ != nullptr) {
            for(const auto &subc : parent_->subcommands_) {
                if(subc.get() != this && (this->check_name(subc->name_) || subc->check_name(this->name_)))
                    throw OptionAlreadyAdded(subc->name_);
            }
        }
        return this;
    }

///check查看是否已分析此子命令，仅当在命令行上接收时才为true。
    bool parsed() const { return parsed_; }

///check查看是否已分析此子命令，仅当在命令行上接收时才为true。
///这允许直接检查子命令。
    operator bool() const { return parsed_; }

///要求提供子命令（不影响帮助调用）
///不返回指针，因为它应该在主应用程序上调用。
    App *require_subcommand(int value = -1) {
        require_subcommand_ = value;
        return this;
    }

///stop子命令fallthrough，这样父命令就不能在子命令之后收集命令。
///default从父级开始，通常在父级设置。
    App *fallthrough(bool value = true) {
        fallthrough_ = value;
        return this;
    }

///@
///@name添加选项
///@

///添加选项，将自动了解常用类型的类型。
///
///若要使用，请创建一个具有预期类型的变量，并在名称后传递它。
///start调用后，可以使用count查看是否传递了值，以及
///该值将正确初始化。支持数字、向量和字符串。
///
///->Required（），->Default，验证程序是选项，
///位置选项接受可选数量的参数。
///
//例如，
///
///std：：字符串文件名；
///program.add_option（“文件名”，文件名，“文件名说明”）；
///
    Option *add_option(std::string name, callback_t callback, std::string description = "", bool defaulted = false) {
        Option myopt{name, description, callback, defaulted, this};

        if(std::find_if(std::begin(options_), std::end(options_), [&myopt](const Option_p &v) {
               return *v == myopt;
           }) == std::end(options_)) {
            options_.emplace_back();
            Option_p &option = options_.back();
            option.reset(new Option(name, description, callback, defaulted, this));
            return option.get();
        } else
            throw OptionAlreadyAdded(myopt.get_name());
    }
    
    

///add非向量选项（需要复制副本而不默认，以避免出现“iostream<<value`”）
    template <typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option *add_option(std::string name,
T &variable, ///<要设置的变量
                       std::string description = "") {

        CLI::callback_t fun = [&variable](CLI::results_t res) {
            if(res.size() != 1)
                return false;
            return detail::lexical_cast(res[0], variable);
        };

        Option *opt = add_option(name, fun, description, false);
        opt->set_custom_option(detail::type_name<T>());
        return opt;
    }

///add选项用于具有默认打印的非矢量
    template <typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option *add_option(std::string name,
T &variable, ///<要设置的变量
                       std::string description,
                       bool defaulted) {
        
        CLI::callback_t fun = [&variable](CLI::results_t res) {
            if(res.size() != 1)
                return false;
            return detail::lexical_cast(res[0], variable);
        };
        
        Option *opt = add_option(name, fun, description, defaulted);
        opt->set_custom_option(detail::type_name<T>());
        if(defaulted) {
            std::stringstream out;
            out << variable;
            opt->set_default_val(out.str());
        }
        return opt;
    }
    
///add向量选项（无默认值）
    template <typename T>
    Option *add_option(std::string name,
std::vector<T> &variable, ///<要设置的变量向量
                       std::string description = "") {
        
        CLI::callback_t fun = [&variable](CLI::results_t res) {
            bool retval = true;
            variable.clear();
            for(const auto &a : res) {
                variable.emplace_back();
                retval &= detail::lexical_cast(a, variable.back());
            }
            return (!variable.empty()) && retval;
        };
        
        Option *opt = add_option(name, fun, description, false);
        opt->set_custom_option(detail::type_name<T>(), -1, true);
        return opt;
    }
    
///add向量选项
    template <typename T>
    Option *add_option(std::string name,
std::vector<T> &variable, ///<要设置的变量向量
                       std::string description,
                       bool defaulted) {

        CLI::callback_t fun = [&variable](CLI::results_t res) {
            bool retval = true;
            variable.clear();
            for(const auto &a : res) {
                variable.emplace_back();
                retval &= detail::lexical_cast(a, variable.back());
            }
            return (!variable.empty()) && retval;
        };

        Option *opt = add_option(name, fun, description, defaulted);
        opt->set_custom_option(detail::type_name<T>(), -1, true);
        if(defaulted)
            opt->set_default_val("[" + detail::join(variable) + "]");
        return opt;
    }

///add标志选项
    Option *add_flag(std::string name, std::string description = "") {
        CLI::callback_t fun = [](CLI::results_t) { return true; };

        Option *opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->set_custom_option("", 0);
        return opt;
    }

///add标志整数选项
    template <typename T,
              enable_if_t<std::is_integral<T>::value && !is_bool<T>::value, detail::enabler> = detail::dummy>
    Option *add_flag(std::string name,
T &count, ///<A varaible holding the count
                     std::string description = "") {

        count = 0;
        CLI::callback_t fun = [&count](CLI::results_t res) {
            count = static_cast<T>(res.size());
            return true;
        };

        Option *opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->set_custom_option("", 0);
        return opt;
    }

///bool版本只允许标志一次
    template <typename T, enable_if_t<is_bool<T>::value, detail::enabler> = detail::dummy>
    Option *add_flag(std::string name,
T &count, ///<A varaible holding true if passed（如果通过，则保持为真）
                     std::string description = "") {

        count = false;
        CLI::callback_t fun = [&count](CLI::results_t res) {
            count = true;
            return res.size() == 1;
        };

        Option *opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->set_custom_option("", 0);
        return opt;
    }

///add选项集（无默认值）
    template <typename T>
    Option *add_set(std::string name,
T &member,           ///<集合的选定成员
std::set<T> options, ///<一组可能性
                    std::string description = "") {

        CLI::callback_t fun = [&member, options](CLI::results_t res) {
            if(res.size() != 1) {
                return false;
            }
            bool retval = detail::lexical_cast(res[0], member);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        Option *opt = add_option(name, fun, description, false);
        std::string typeval = detail::type_name<T>();
        typeval += " in {" + detail::join(options) + "}";
        opt->set_custom_option(typeval);
        return opt;
    }
    
///添加选项集
    template <typename T>
    Option *add_set(std::string name,
T &member,           ///<集合的选定成员
std::set<T> options, ///<一组可能性
                    std::string description,
                    bool defaulted) {
        
        CLI::callback_t fun = [&member, options](CLI::results_t res) {
            if(res.size() != 1) {
                return false;
            }
            bool retval = detail::lexical_cast(res[0], member);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };
        
        Option *opt = add_option(name, fun, description, defaulted);
        std::string typeval = detail::type_name<T>();
        typeval += " in {" + detail::join(options) + "}";
        opt->set_custom_option(typeval);
        if(defaulted) {
            std::stringstream out;
            out << member;
            opt->set_default_val(out.str());
        }
        return opt;
    }

///add选项集，仅字符串，忽略大小写（无默认值）
    Option *add_set_ignore_case(std::string name,
std::string &member,           ///<集合的选定成员
std::set<std::string> options, ///<一组可能性
                                std::string description = "") {

        CLI::callback_t fun = [&member, options](CLI::results_t res) {
            if(res.size() != 1) {
                return false;
            }
            member = detail::to_lower(res[0]);
            auto iter = std::find_if(std::begin(options), std::end(options), [&member](std::string val) {
                return detail::to_lower(val) == member;
            });
            if(iter == std::end(options))
                return false;
            else {
                member = *iter;
                return true;
            }
        };

        Option *opt = add_option(name, fun, description, false);
        std::string typeval = detail::type_name<std::string>();
        typeval += " in {" + detail::join(options) + "}";
        opt->set_custom_option(typeval);
        
        return opt;
    }
    
///add选项集，仅字符串，忽略大小写
    Option *add_set_ignore_case(std::string name,
std::string &member,           ///<集合的选定成员
std::set<std::string> options, ///<一组可能性
                                std::string description,
                                bool defaulted) {
        
        CLI::callback_t fun = [&member, options](CLI::results_t res) {
            if(res.size() != 1) {
                return false;
            }
            member = detail::to_lower(res[0]);
            auto iter = std::find_if(std::begin(options), std::end(options), [&member](std::string val) {
                return detail::to_lower(val) == member;
            });
            if(iter == std::end(options))
                return false;
            else {
                member = *iter;
                return true;
            }
        };
        
        Option *opt = add_option(name, fun, description, defaulted);
        std::string typeval = detail::type_name<std::string>();
        typeval += " in {" + detail::join(options) + "}";
        opt->set_custom_option(typeval);
        if(defaulted) {
            opt->set_default_val(member);
        }
        return opt;
    }

///添加复数
    template <typename T>
    Option *add_complex(std::string name,
                        T &variable,
                        std::string description = "",
                        bool defaulted = false,
                        std::string label = "COMPLEX") {
        CLI::callback_t fun = [&variable](results_t res) {
            if(res.size() != 2)
                return false;
            double x, y;
            bool worked = detail::lexical_cast(res[0], x) && detail::lexical_cast(res[1], y);
            if(worked)
                variable = T(x, y);
            return worked;
        };

        CLI::Option *opt = add_option(name, fun, description, defaulted);
        opt->set_custom_option(label, 2);
        if(defaulted) {
            std::stringstream out;
            out << variable;
            opt->set_default_val(out.str());
        }
        return opt;
    }

///添加配置ini文件选项
    Option *add_config(std::string name = "--config",
                       std::string default_filename = "",
                       std::string help = "Read an ini file",
                       bool required = false) {

//删除现有配置（如果存在）
        if(config_ptr_ != nullptr)
            remove_option(config_ptr_);
        config_name_ = default_filename;
        config_required_ = required;
        config_ptr_ = add_option(name, config_name_, help, default_filename != "");
        return config_ptr_;
    }

///从应用程序中删除一个选项。采用选项指针。如果找到并删除，则返回true。
    bool remove_option(Option *opt) {
        auto iterator =
            std::find_if(std::begin(options_), std::end(options_), [opt](const Option_p &v) { return v.get() == opt; });
        if(iterator != std::end(options_)) {
            options_.erase(iterator);
            return true;
        }
        return false;
    }

///@
///@name子命令
///@

///添加子命令。与构造函数类似，您可以通过设置help=false来重写帮助消息添加。
    App *add_subcommand(std::string name, std::string description = "", bool help = true) {
        subcommands_.emplace_back(new App(description, help, detail::dummy));
        subcommands_.back()->name_ = name;
        subcommands_.back()->allow_extras();
        subcommands_.back()->parent_ = this;
        subcommands_.back()->ignore_case_ = ignore_case_;
        subcommands_.back()->fallthrough_ = fallthrough_;
        for(const auto &subc : subcommands_)
            if(subc.get() != subcommands_.back().get())
                if(subc->check_name(subcommands_.back()->name_) || subcommands_.back()->check_name(subc->name_))
                    throw OptionAlreadyAdded(subc->name_);
        return subcommands_.back().get();
    }

///check查看子命令是否是此命令的一部分（不必在命令行中）
    App *get_subcommand(App *subcom) const {
        for(const App_p &subcomptr : subcommands_)
            if(subcomptr.get() == subcom)
                return subcom;
        throw CLI::OptionNotFound(subcom->get_name());
    }

///check查看子命令是否是此命令的一部分（文本版本）
    App *get_subcommand(std::string subcom) const {
        for(const App_p &subcomptr : subcommands_)
            if(subcomptr->check_name(subcom))
                return subcomptr.get();
        throw CLI::OptionNotFound(subcom);
    }

///@
///@name用于子类化的附加项
///@

///这允许子类在回调之前但在解析之后插入代码。
///
///This is not run if any errors or help is through.
    virtual void pre_callback() {}

///@
//名称解析
///@

///分析命令行-引发错误
///this必须在选项进入后但在程序其余部分之前调用。
    std::vector<std::string> parse(int argc, char **argv) {
        name_ = argv[0];
        std::vector<std::string> args;
        for(int i = argc - 1; i > 0; i--)
            args.emplace_back(argv[i]);
        return parse(args);
    }

///真正的工作在这里完成。应为反向矢量。
///将矢量更改为其余选项。
    std::vector<std::string> &parse(std::vector<std::string> &args) {
        _validate();
        _parse(args);
        run_callback();
        return args;
    }

///打印错误消息并返回退出代码
    int exit(const Error &e) const {
        if(e.exit_code != static_cast<int>(ExitCodes::Success)) {
            std::cerr << "ERROR: ";
            std::cerr << e.what() << std::endl;
            if(e.print_help)
                std::cerr << help();
        } else {
            if(e.print_help)
                std::cout << help();
        }
        return e.get_exit_code();
    }

///重置分析的数据
    void reset() {

        parsed_ = false;
        missing_.clear();

        for(const Option_p &opt : options_) {
            opt->clear();
        }
        for(const App_p &app : subcommands_) {
            app->reset();
        }
    }

///@
///@name后解析
///@

///统计给定选项被传递的次数。
    size_t count(std::string name) const {
        for(const Option_p &opt : options_) {
            if(opt->check_name(name)) {
                return opt->count();
            }
        }
        throw OptionNotFound(name);
    }

///get指向当前所选子命令的子命令指针列表（分析后）
    std::vector<App *> get_subcommands() const {
        std::vector<App *> subcomms;
        for(const App_p &subcomptr : subcommands_)
            if(subcomptr->parsed_)
                subcomms.push_back(subcomptr.get());
        return subcomms;
    }

///check查看是否选择了给定的子命令
    bool got_subcommand(App *subcom) const {
//需要get subcomm来验证这是一个真正的子命令
        return get_subcommand(subcom)->parsed_;
    }

///check用name而不是指针查看是否选择了子命令
    bool got_subcommand(std::string name) const { return get_subcommand(name)->parsed_; }

///@
//@名称帮助
///@

///生成一个可以作为应用程序当前值的配置读取的字符串。设置默认值也包括
///默认参数。前缀将在每个选项的开头添加一个字符串。
    std::string config_to_str(bool default_also = false, std::string prefix = "") const {
        std::stringstream out;
        for(const Option_p &opt : options_) {

//只有长名称的进程选项
            if(!opt->lnames_.empty()) {
                std::string name = prefix + opt->lnames_[0];

//非旗
                if(opt->get_expected() != 0) {

//如果在命令行上找到该选项
                    if(opt->count() > 0)
                        out << name << "=" << detail::inijoin(opt->results()) << std::endl;

//如果选项具有默认值并且由可选参数请求
                    else if(default_also && opt->defaultval_ != "")
                        out << name << "=" << opt->defaultval_ << std::endl;
//标志，一个通过
                } else if(opt->count() == 1) {
                    out << name << "=true" << std::endl;

//标志，多次通过
                } else if(opt->count() > 1) {
                    out << name << "=" << opt->count() << std::endl;

//标志，不存在
                } else if(opt->count() == 0 && default_also && opt.get() != get_help_ptr()) {
                    out << name << "=false" << std::endl;
                }
            }
        }
        for(const App_p &subcom : subcommands_)
            out << subcom->config_to_str(default_also, prefix + subcom->name_ + ".");
        return out.str();
    }

///makes a help message，column wid for column 1
    std::string help(size_t wid = 30, std::string prev = "") const {
//如果需要，委托给子命令
        if(prev == "")
            prev = name_;
        else
            prev += " " + name_;

        auto selected_subcommands = get_subcommands();
        if(!selected_subcommands.empty())
            return selected_subcommands.at(0)->help(wid, prev);

        std::stringstream out;
        out << description_ << std::endl;
        out << "Usage: " << prev;

//检查选项\u
        bool npos = false;
        std::set<std::string> groups;
        for(const Option_p &opt : options_) {
            if(opt->nonpositional()) {
                npos = true;
                groups.insert(opt->get_group());
            }
        }

        if(npos)
            out << " [OPTIONS]";

//定位器
        bool pos = false;
        for(const Option_p &opt : options_)
            if(opt->get_positional()) {
//隐藏位置仍应显示在usage语句中
//if（detail:：to_lower（opt->get_group（））=隐藏）
//继续；
                out << " " << opt->help_positional();
                if(opt->_has_help_positional())
                    pos = true;
            }

        if(!subcommands_.empty()) {
            if(require_subcommand_ != 0)
                out << " SUBCOMMAND";
            else
                out << " [SUBCOMMAND]";
        }

        out << std::endl << std::endl;

//位置描述
        if(pos) {
            out << "Positionals:" << std::endl;
            for(const Option_p &opt : options_) {
                if(detail::to_lower(opt->get_group()) == "hidden")
                    continue;
                if(opt->_has_help_positional())
                    detail::format_help(out, opt->help_pname(), opt->get_description(), wid);
            }
            out << std::endl;
        }

//选项
        if(npos) {
            for(const std::string &group : groups) {
                if(detail::to_lower(group) == "hidden")
                    continue;
                out << group << ":" << std::endl;
                for(const Option_p &opt : options_) {
                    if(opt->nonpositional() && opt->get_group() == group)
                        detail::format_help(out, opt->help_name(), opt->get_description(), wid);
                }
                out << std::endl;
            }
        }

//子命令
        if(!subcommands_.empty()) {
            out << "Subcommands:" << std::endl;
            for(const App_p &com : subcommands_)
                detail::format_help(out, com->get_name(), com->description_, wid);
        }
        return out.str();
    }

///@
//@命名吸气剂
///@

///获取指向帮助标志的指针。
    Option *get_help_ptr() { return help_ptr_; }

///获取指向帮助标志的指针。（康斯特）
    const Option *get_help_ptr() const { return help_ptr_; }

///get指向配置选项的指针。
    Option *get_config_ptr() { return config_ptr_; }

///get指向配置选项的指针。（康斯特）
    const Option *get_config_ptr() const { return config_ptr_; }
///获取当前应用程序的名称
    std::string get_name() const { return name_; }

///检查名称，如果设置不区分大小写
    bool check_name(std::string name_to_check) const {
        std::string local_name = name_;
        if(ignore_case_) {
            local_name = detail::to_lower(name_);
            name_to_check = detail::to_lower(name_to_check);
        }

        return local_name == name_to_check;
    }

///this获取具有原始分析顺序的指针向量。
    const std::vector<Option *> &parse_order() const { return parse_order_; }

///@

  protected:
///检查选项以确保没有配置。
///
///currenly检查是否存在带有-1个参数的多个位置
    void _validate() const {
        auto count = std::count_if(std::begin(options_), std::end(options_), [](const Option_p &opt) {
            return opt->get_expected() == -1 && opt->get_positional();
        });
        if(count > 1)
            throw InvalidError(name_ + ": Too many positional arguments with unlimited expected args");
        for(const App_p &app : subcommands_)
            app->_validate();
    }

///主控形状中缺少返回
    missing_t *missing() {
        if(parent_ != nullptr)
            return parent_->missing();
        return &missing_;
    }

///internal函数运行（app）回调，自上而下
    void run_callback() {
        pre_callback();
        if(callback_)
            callback_();
        for(App *subc : get_subcommands()) {
            subc->run_callback();
        }
    }

    bool _valid_subcommand(const std::string &current) const {
        for(const App_p &com : subcommands_)
            if(com->check_name(current) && !*com)
                return true;
        if(parent_ != nullptr)
            return parent_->_valid_subcommand(current);
        else
            return false;
    }

///根据当前参数的类型选择分类器枚举
    detail::Classifer _recognize(const std::string &current) const {
        std::string dummy1, dummy2;

        if(current == "--")
            return detail::Classifer::POSITIONAL_MARK;
        if(_valid_subcommand(current))
            return detail::Classifer::SUBCOMMAND;
        if(detail::split_long(current, dummy1, dummy2))
            return detail::Classifer::LONG;
        if(detail::split_short(current, dummy1, dummy2))
            return detail::Classifer::SHORT;
        return detail::Classifer::NONE;
    }

///内部分析函数
    void _parse(std::vector<std::string> &args) {
        parsed_ = true;
        bool positional_only = false;

        while(!args.empty()) {
            _parse_single(args, positional_only);
        }

        if(help_ptr_ != nullptr && help_ptr_->count() > 0) {
            throw CallForHelp();
        }

//处理ini文件
        if(config_ptr_ != nullptr && config_name_ != "") {
            try {
                std::vector<detail::ini_ret_t> values = detail::parse_ini(config_name_);
                while(!values.empty()) {
                    if(!_parse_ini(values)) {
                        throw ExtrasINIError(values.back().fullname);
                    }
                }
            } catch(const FileError &) {
                if(config_required_)
                    throw;
            }
        }

//获取envname选项（如果尚未传递）
        for(const Option_p &opt : options_) {
            if(opt->count() == 0 && opt->envname_ != "") {
                char *buffer = nullptr;
                std::string ename_string;

                #ifdef _MSC_VER
//Windows版本
                size_t sz = 0;
                if(_dupenv_s(&buffer, &sz, opt->envname_.c_str()) == 0 && buffer != nullptr) {
                    ename_string = std::string(buffer);
                    free(buffer);
                }
                #else
//这也适用于Windows，但会发出警告
                buffer = std::getenv(opt->envname_.c_str());
                if(buffer != nullptr)
                    ename_string = std::string(buffer);
                #endif

                if(!ename_string.empty()) {
                    opt->add_result(ename_string);
                }
            }
        }

//处理回调
        for(const Option_p &opt : options_) {
            if(opt->count() > 0 && !opt->get_callback_run()) {
                opt->run_callback();
            }
        }

//验证所需选项
        for(const Option_p &opt : options_) {
//要求的
            if(opt->get_required() && (static_cast<int>(opt->count()) < opt->get_expected() || opt->count() == 0))
                throw RequiredError(opt->get_name());
//要求
            for(const Option *opt_req : opt->requires_)
                if(opt->count() > 0 && opt_req->count() == 0)
                    throw RequiresError(opt->get_name(), opt_req->get_name());
//排除
            for(const Option *opt_ex : opt->excludes_)
                if(opt->count() > 0 && opt_ex->count() != 0)
                    throw ExcludesError(opt->get_name(), opt_ex->get_name());
        }

        auto selected_subcommands = get_subcommands();
        if(require_subcommand_ < 0 && selected_subcommands.empty())
            throw RequiredError("Subcommand required");
        else if(require_subcommand_ > 0 && static_cast<int>(selected_subcommands.size()) != require_subcommand_)
            throw RequiredError(std::to_string(require_subcommand_) + " subcommand(s) required");

//将缺少的（对）转换为额外项（仅字符串）
        if(parent_ == nullptr) {
            args.resize(missing()->size());
            std::transform(std::begin(*missing()),
                           std::end(*missing()),
                           std::begin(args),
                           [](const std::pair<detail::Classifer, std::string> &val) { return val.second; });
            std::reverse(std::begin(args), std::end(args));

            size_t num_left_over = std::count_if(
                std::begin(*missing()), std::end(*missing()), [](std::pair<detail::Classifer, std::string> &val) {
                    return val.first != detail::Classifer::POSITIONAL_MARK;
                });

            if(num_left_over > 0 && !(allow_extras_ || prefix_command_))
                throw ExtrasError("[" + detail::rjoin(args, " ") + "]");
        }
    }

///parse one ini参数，如果在任何子命令中找不到，则返回false；如果找到，则删除。
///
///如果它有多个dot.separated.name，请转到与之匹配的子命令。
///如果成功找到选项，则返回true；如果为false，则需要手动删除arg。
    bool _parse_ini(std::vector<detail::ini_ret_t> &args) {
        detail::ini_ret_t &current = args.back();
std::string parent = current.parent(); //尊重当前级别
        std::string name = current.name();

//如果列出了父级，请转到子命令
        if(parent != "") {
            current.level++;
            for(const App_p &com : subcommands_)
                if(com->check_name(parent))
                    return com->_parse_ini(args);
            return false;
        }

        auto op_ptr = std::find_if(
            std::begin(options_), std::end(options_), [name](const Option_p &v) { return v->check_lname(name); });

        if(op_ptr == std::end(options_))
            return false;

//我们不要对指针语法发疯
        Option_p &op = *op_ptr;

        if(op->results_.empty()) {
//标志解析
            if(op->get_expected() == 0) {
                if(current.inputs.size() == 1) {
                    std::string val = current.inputs.at(0);
                    val = detail::to_lower(val);
                    if(val == "true" || val == "on" || val == "yes")
                        op->results_ = {""};
                    else if(val == "false" || val == "off" || val == "no")
                        ;
                    else
                        try {
                            size_t ui = std::stoul(val);
                            for(size_t i = 0; i < ui; i++)
                                op->results_.emplace_back("");
                        } catch(const std::invalid_argument &) {
                            throw ConversionError(current.fullname + ": Should be true/false or a number");
                        }
                } else
                    throw ConversionError(current.fullname + ": too many inputs for a flag");
            } else {
                op->results_ = current.inputs;
                op->run_callback();
            }
        }

        args.pop_back();
        return true;
    }

///parse“one”参数（有些参数可能会吃掉多个参数），如果失败则委托给父级，如果缺少则添加到缺少的。
//大师
    void _parse_single(std::vector<std::string> &args, bool &positional_only) {

        detail::Classifer classifer = positional_only ? detail::Classifer::NONE : _recognize(args.back());
        switch(classifer) {
        case detail::Classifer::POSITIONAL_MARK:
            missing()->emplace_back(classifer, args.back());
            args.pop_back();
            positional_only = true;
            break;
        case detail::Classifer::SUBCOMMAND:
            _parse_subcommand(args);
            break;
        case detail::Classifer::LONG:
//如果已经分析了子命令，则不接受选项
            _parse_long(args);
            break;
        case detail::Classifer::SHORT:
//如果已经分析了子命令，则不接受选项
            _parse_short(args);
            break;
        case detail::Classifer::NONE:
//可能是父（子）命令的位置命令或其他命令
            _parse_positional(args);
        }
    }

///count所需的剩余位置参数
    size_t _count_remaining_required_positionals() const {
        size_t retval = 0;
        for(const Option_p &opt : options_)
            if(opt->get_positional()
               && opt->get_required()
               && opt->get_expected() > 0
               && static_cast<int>(opt->count()) < opt->get_expected())
                retval = static_cast<size_t>(opt->get_expected()) - opt->count();

        return retval;
    }

///分析位置，向上树检查
    void _parse_positional(std::vector<std::string> &args) {

        std::string positional = args.back();
        for(const Option_p &opt : options_) {
//一个接一个吃，直到吃完
            if(opt->get_positional() &&
               (static_cast<int>(opt->count()) < opt->get_expected() || opt->get_expected() < 0)) {

                opt->add_result(positional);
                parse_order_.push_back(opt.get());
                args.pop_back();
                return;
            }
        }

        if(parent_ != nullptr && fallthrough_)
            return parent_->_parse_positional(args);
        else {
            args.pop_back();
            missing()->emplace_back(detail::Classifer::NONE, positional);
            
            if(prefix_command_) {
                while(!args.empty()) {
                    missing()->emplace_back(detail::Classifer::NONE, args.back());
                    args.pop_back();
                }
            }
        }
        
    }

///parse子命令，修改参数并继续
///
///与其他人不同，这一个总是允许通过
    void _parse_subcommand(std::vector<std::string> &args) {
        if(_count_remaining_required_positionals() > 0)
            return _parse_positional(args);
        for(const App_p &com : subcommands_) {
            if(com->check_name(args.back())) {
                args.pop_back();
                com->_parse(args);
                return;
            }
        }
        if(parent_ != nullptr)
            return parent_->_parse_subcommand(args);
        else
            throw HorribleError("Subcommand " + args.back() + " missing");
    }

///parse一个短参数，必须在列表的顶部
    void _parse_short(std::vector<std::string> &args) {
        std::string current = args.back();

        std::string name;
        std::string rest;
        if(!detail::split_short(current, name, rest))
            throw HorribleError("Short");

        auto op_ptr = std::find_if(
            std::begin(options_), std::end(options_), [name](const Option_p &opt) { return opt->check_sname(name); });

//找不到选项
        if(op_ptr == std::end(options_)) {
//如果是子命令，请尝试master命令
            if(parent_ != nullptr && fallthrough_)
                return parent_->_parse_short(args);
//否则，添加到缺少的
            else {
                args.pop_back();
                missing()->emplace_back(detail::Classifer::SHORT, current);
                return;
            }
        }

        args.pop_back();

//获取对指针的引用以使语法可承受
        Option_p &op = *op_ptr;

        int num = op->get_expected();

        if(num == 0) {
            op->add_result("");
            parse_order_.push_back(op.get());
        } else if(rest != "") {
            if(num > 0)
                num--;
            op->add_result(rest);
            parse_order_.push_back(op.get());
            rest = "";
        }

        if(num == -1) {
            while(!args.empty() && _recognize(args.back()) == detail::Classifer::NONE) {
                op->add_result(args.back());
                parse_order_.push_back(op.get());
                args.pop_back();
            }
        } else
            while(num > 0 && !args.empty()) {
                num--;
                std::string current_ = args.back();
                args.pop_back();
                op->add_result(current_);
                parse_order_.push_back(op.get());
            }

        if(rest != "") {
            rest = "-" + rest;
            args.push_back(rest);
        }
    }

///parse一个长参数，必须在列表的顶部
    void _parse_long(std::vector<std::string> &args) {
        std::string current = args.back();

        std::string name;
        std::string value;
        if(!detail::split_long(current, name, value))
            throw HorribleError("Long:" + args.back());

        auto op_ptr = std::find_if(
            std::begin(options_), std::end(options_), [name](const Option_p &v) { return v->check_lname(name); });

//找不到选项
        if(op_ptr == std::end(options_)) {
//如果是子命令，请尝试master命令
            if(parent_ != nullptr && fallthrough_)
                return parent_->_parse_long(args);
//否则，添加到缺少的
            else {
                args.pop_back();
                missing()->emplace_back(detail::Classifer::LONG, current);
                return;
            }
        }

        args.pop_back();

//获取对指针的引用以使语法可承受
        Option_p &op = *op_ptr;

        int num = op->get_expected();

        if(value != "") {
            if(num != -1)
                num--;
            op->add_result(value);
            parse_order_.push_back(op.get());
        } else if(num == 0) {
            op->add_result("");
            parse_order_.push_back(op.get());
        }

        if(num == -1) {
            while(!args.empty() && _recognize(args.back()) == detail::Classifer::NONE) {
                op->add_result(args.back());
                parse_order_.push_back(op.get());
                args.pop_back();
            }
        } else
            while(num > 0 && !args.empty()) {
                num--;
                op->add_result(args.back());
                parse_order_.push_back(op.get());
                args.pop_back();
            }
        return;
    }
};

namespace detail {
///此类只允许测试访问应用程序的受保护函数。
struct AppFriend {

///wrap _解析\短的、完全向前的参数并返回
    template <typename... Args>
    static auto parse_short(App *app, Args &&... args) ->
        typename std::result_of<decltype (&App::_parse_short)(App, Args...)>::type {
        return app->_parse_short(std::forward<Args>(args)...);
    }

///wrap \解析\长的、完全向前的参数并返回
    template <typename... Args>
    static auto parse_long(App *app, Args &&... args) ->
        typename std::result_of<decltype (&App::_parse_long)(App, Args...)>::type {
        return app->_parse_long(std::forward<Args>(args)...);
    }

///wrap _parse_子命令，完美地转发参数并返回
    template <typename... Args>
    static auto parse_subcommand(App *app, Args &&... args) ->
        typename std::result_of<decltype (&App::_parse_subcommand)(App, Args...)>::type {
        return app->_parse_subcommand(std::forward<Args>(args)...);
    }
};
} //命名空间详细信息

} //命名空间CLI
