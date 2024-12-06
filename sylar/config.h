#ifndef __SYLAR__CONFIG_H__
#define __SYLAR__CONFIG_H__

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "log.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/parser.h>
#include <algorithm>
#include <string>
namespace sylar
{
    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string &name, const std::string &description = "") : m_name(name),
                                                                                      m_description{description}
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase() {};
        const std::string &getName() const
        {
            return this->m_name;
        }

        const std::string &getDescription() const { return this->m_description; }

        // 转为string
        virtual std::string toString() = 0;

        // 从string转
        virtual bool fromString(const std::string &val) = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };
    // FORM TO
    template <class F, class T>
    class LexicalCast
    {
    public:
        T operator()(const F &v)
        {
            return boost::lexical_cast<T>(v);
        }
    };

    // 对vector类型做偏特化
    // 从str到vector
    template <class T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::vector<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };
    // 从vector到str
    template <class T>
    class LexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // 序列化和反序列化
    // FromStr T operator()(const std::string&)
    // ToStr std::string operator()(const T&)
    template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        ConfigVar(const std::string &name, const T &default_value, const std::string &description) : ConfigVarBase(name, description),
                                                                                                     m_value(default_value)
        {
        }
        std::string toString() override
        {
            try
            {
                // lexical_cast函数模板提供了一种方便且一致的形式，用于支持以文本形式表示的任意类型之间的公共转换
                // return boost::lexical_cast<std::string>(m_value);
                return ToStr()(m_value);
            }
            catch (std::exception &e)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() << "convert:" << typeid(m_value).name() << "to_string";
                return "";
            }
        }
        bool fromString(const std::string &val) override
        {
            try
            {
                // m_value = boost::lexical_cast<T>(val);
                setValue(FromStr()(val));
                return true;
            }
            catch (std::exception &e)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what() << "convert string to" << typeid(m_value).name() << "from string";
            }
            return false;
        }
        const T getValue() const
        {
            return m_value;
        }

        void setValue(const T &t)
        {
            m_value = t;
        }

    private:
        T m_value;
    };

    // 模板类不能在cpp中实现，因为模板类在运行时才实现，所以连接的时候如果分离，就会报错
    class Config
    {
    public:
        // s_datas数据类型

        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name, const T &default_value, const std::string &description)
        {
            auto tmp = Lookup<T>(name);
            if (tmp)
            {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "LookUp name=" << name << "exists";
                return tmp;
            }
            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid" << name;
                throw std::invalid_argument(name);
            }
            // 显示说明这是一个ConfigVar<T>::ptr是类型而不是成员变量
            // 防止出现configvar中有ptr变量
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            s_datas[name] = v;
            return v;
        }
        //
        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name)
        {
            auto it = s_datas.find(name);
            if (it == s_datas.end())
            {
                return nullptr;
            }
            //
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }

        static void LoadFromYaml(const YAML::Node &root);
        static ConfigVarBase::ptr LookupBase(const std::string &name);

    private:
        static ConfigVarMap s_datas;
    };

} // namespace sylar

#endif