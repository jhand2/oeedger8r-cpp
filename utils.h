// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#ifndef UTILS_H
#define UTILS_H

#include <ostream>
#include <sstream>
#include <string>

#include "ast.h"

template <typename A, typename C>
bool in(const A& a, const C& c)
{
    return std::find(c.begin(), c.end(), a) != c.end();
}

template <typename C1, typename C2>
void append(C1& c1, const C2& c2)
{
    c1.insert(c1.end(), c2.begin(), c2.end());
}

template <typename T>
void autogen_preamble(T& os)
{
    os << "/*"
       << " *  This file is auto generated by oeedger8r. DO NOT EDIT."
       << " */";
}

template <typename T>
void header(T& os, const std::string& guard)
{
    autogen_preamble(os);
    // clang-format off
    os << "#ifndef " + guard
       << "#define " + guard;
    // clang-format on
}

template <typename T>
void footer(T& os, const std::string& guard)
{
    os << "#endif // " + guard;
}

inline std::string upper(const std::string& s)
{
    std::string t(s);
    for (char& ch : t)
        ch = static_cast<char>(toupper(ch));
    return t;
}

inline std::string atype_str(Type* t)
{
    if (t->tag_ == Enum)
        return "enum " + t->name_;
    if (t->tag_ == Struct)
        return "struct " + t->name_;
    if (t->tag_ == Union)
        return "union " + t->name_;
    if (t->tag_ == Const)
        return "const " + atype_str(t->t_);
    if (t->tag_ == Short)
        return "short int";
    if (t->tag_ == Long)
        return "long int";
    if (t->tag_ == Unsigned)
        return "unsigned " + atype_str(t->t_);
    if (t->tag_ == Ptr)
        return atype_str(t->t_) + "*";
    if (t->tag_ == Foreign)
        return t->name_;

#define TYPE_STR(tag, tstr) \
    if (t->tag_ == tag)     \
    return tstr

    TYPE_STR(Bool, "bool");
    TYPE_STR(Char, "char");
    TYPE_STR(Int, "int");
    TYPE_STR(LLong, "long long");
    TYPE_STR(Float, "float");
    TYPE_STR(Double, "double");
    TYPE_STR(LDouble, "long double");
    TYPE_STR(Int8, "int8_t");
    TYPE_STR(Int16, "int16_t");
    TYPE_STR(Int32, "int32_t");
    TYPE_STR(Int64, "int64_t");
    TYPE_STR(UInt8, "uint8_t");
    TYPE_STR(UInt16, "uint16_t");
    TYPE_STR(UInt32, "uint32_t");
    TYPE_STR(UInt64, "uint64_t");
    TYPE_STR(WChar, "wchar_t");
    TYPE_STR(Void, "void");
    TYPE_STR(SizeT, "size_t");

    return "";
}

inline std::string dims_str(Dims* dims, size_t idx = 0)
{
    if (dims && idx < dims->size())
        return "[" + dims->at(idx) + "]" + dims_str(dims, idx + 1);
    return "";
}

inline std::string decl_str(const std::string& name, Type* t, Dims* dims)
{
    if (std::isalpha(name[0]) || name[0] == '_')
        return atype_str(t) + " " + name + dims_str(dims);
    else
        return atype_str(t) + name + dims_str(dims);
}

inline std::string replace(
    const std::string s,
    const std::string& p,
    const std::string& q)
{
    std::string::size_type start_pos = s.find(p);
    if (start_pos == std::string::npos)
        return s;
    return s.substr(0, start_pos) + q + s.substr(start_pos + p.length());
}

inline std::string mtype_str(Decl* p)
{
    if (p->type_->tag_ == Foreign && p->attrs_ && p->attrs_->isary_)
        return "/* foreign array of type " + p->type_->name_ + " */ void*";
    std::string s = atype_str(p->type_) + (p->dims_ ? "*" : "");
    return replace(s, "const ", "");
}

inline std::string mdecl_str(
    const std::string& name,
    Type* t,
    Dims* dims,
    Attrs* attrs)
{
    if (t->tag_ == Foreign && attrs && attrs->isary_)
        return "/* foreign array of type " + t->name_ + " */ void* " + name;
    std::string decl = atype_str(t) + (dims ? "* " : " ") + name;
    return replace(decl, "const ", "");
}

inline std::string prototype(
    const Function* f,
    bool ecall = true,
    bool gen_t = true)
{
    std::string retstr =
        (ecall != gen_t) ? "oe_result_t" : atype_str(f->rtype_);

    std::vector<std::string> args;
    if (ecall && !gen_t)
        args.push_back("oe_enclave_t* enclave");

    if (ecall != gen_t && f->rtype_->tag_ != Void)
        args.push_back(atype_str(f->rtype_) + "* _retval");

    for (Decl* p : f->params_)
        args.push_back(decl_str(p->name_, p->type_, p->dims_));

    std::string argsstr;
    if (args.empty())
        argsstr = gen_t && !ecall ? "(\n    )" : "(void)";
    else if (args.size() == 1)
        argsstr = "(" + args[0] + ")";
    else
    {
        argsstr = "(\n    " + args[0];
        for (size_t i = 1; i < args.size(); ++i)
            argsstr += ",\n    " + args[i];
        argsstr += ")";
    }
    return retstr + " " + f->name_ + argsstr;
}

inline std::string create_prototype(const std::string& ename)
{
    return "oe_result_t oe_create_" + ename + "_enclave(\n" +
           "    const char* path,\n"
           "    oe_enclave_type_t type,\n"
           "    uint32_t flags,\n"
           "    const oe_enclave_setting_t* settings,\n"
           "    uint32_t setting_count,\n"
           "    oe_enclave_t** enclave)";
}

inline std::string btype(Type* t)
{
    if (t->tag_ == Const || t->tag_ == Ptr)
        return btype(t->t_);
    if (t->tag_ == Foreign)
        return t->name_;
    if (t->tag_ == Enum)
        return "enum " + t->name_;
    if (t->tag_ == Struct)
        return "struct " + t->name_;
    if (t->tag_ == Union)
        return "union " + t->name_;
    return atype_str(t);
}

inline std::string count_attr_str(
    const Token& t,
    const std::string& prefix = "")
{
    if (t.is_name())
        return prefix + static_cast<std::string>(t);
    else
        return t;
}

inline std::string size_attr_str(const Token& t, const std::string& prefix = "")
{
    return count_attr_str(t, prefix);
}

inline std::string psize(Decl* p, const std::string& prefix = "")
{
    if (p->attrs_ && (p->attrs_->string_ || p->attrs_->wstring_))
        return prefix + p->name_ + "_len * sizeof(" + btype(p->type_) + ")";
    if (p->dims_ && !p->dims_->empty())
        return "sizeof(" + decl_str("", p->type_, p->dims_) + ")";
    if (p->type_->tag_ == Foreign && p->attrs_ && p->attrs_->isary_)
        return "sizeof(" + p->type_->name_ + ")";

    std::string s = "";
    if (p->type_->tag_ == Ptr)
        s = "sizeof(" + replace(atype_str(p->type_->t_), "const ", "") + ")";
    else if (p->type_->tag_ == Foreign && p->attrs_ && p->attrs_->isptr_)
        s = "sizeof(*(" + p->type_->name_ + ")0)";

    if (!p->attrs_->size_.is_empty() && !p->attrs_->count_.is_empty())
        return "(" + size_attr_str(p->attrs_->size_, prefix) + " * " +
               count_attr_str(p->attrs_->count_, prefix) + ")";

    if (p->attrs_ && !p->attrs_->count_.is_empty())
        return "((size_t)" + count_attr_str(p->attrs_->count_, prefix) + " * " +
               s + ")";

    if (p->attrs_ && !p->attrs_->size_.is_empty())
        return size_attr_str(p->attrs_->size_, prefix);

    return s;
}

template <typename T>
inline std::string to_str(const T& t)
{
    std::ostringstream os;
    os << t;
    return os.str();
}

inline UserType* get_user_type(
    const std::vector<UserType*>& types,
    const std::string& name)
{
    for (UserType* t : types)
    {
        if (t->name_ == name)
            return t;
    }
    return nullptr;
}

inline UserType* get_user_type(Edl* edl, const std::string& name)
{
    return get_user_type(edl->types_, name);
}

template <typename Action>
void iterate_deep_copyable_fields(UserType* user_type, Action&& action)
{
    if (user_type->tag_ != Struct)
        return;

    for (Decl* prop : user_type->fields_)
    {
        if (prop->attrs_)
            action(prop);
    }
}

inline UserType* get_user_type_for_deep_copy(Edl* edl, Decl* d)
{
    Type* t = d->type_;
    UserType* ut = nullptr;
    if (t->tag_ != Ptr || !d->attrs_)
        return nullptr;

    // Unwrap first level * and const.
    t = t->t_;
    if (t->tag_ == Const)
        t = t->t_;

    // EDL types can masquerade as foregin types. Depends on what
    // the parser wants to do.
    if (t->tag_ == Foreign || t->tag_ == Struct)
        ut = get_user_type(edl, t->name_);
    if (!ut)
        return nullptr;

    bool deep_copyable = false;
    iterate_deep_copyable_fields(
        ut, [&deep_copyable](Decl*) { deep_copyable = true; });

    return deep_copyable ? ut : nullptr;
}

#endif // UTILS_H
