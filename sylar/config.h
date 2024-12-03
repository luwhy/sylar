#ifndef __SYLAR__CONFIG_H__
#define __SYLAR__CONFIG_H__

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "log.h"
namespace sylar
{
    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string &name, const std::string &description = "") : m_name(name),
                                                                                      m_description(description)
        {
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

    template <class T>
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
                boost::lexical_cast<std::string>(m_value);
            }
            catch (std::exception &e)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() << "convert:" << typeid(m_value).name() << "to_string";
            }
            return "";
        }
        bool fromString(const std::string &val) override
        {
            try
            {
                m_value = boost::lexical_cast<T>(val);
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
            if (name.find_first_not_of("abdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789") != std::string::npos)
            {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid" << name;
                throw std::invalid_argument(name);
            }
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

    private:
        static ConfigVarMap s_datas;
    };

} // namespace sylar

#endif