#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#include "Consts.h"

#include <cstdint>
#include <string>

class UplayConfig
{
    std::string m_Username = CONSTS::DEFAULT_USERNAME;
    std::string m_UserId = CONSTS::DEFAULT_USER_ID;
    uint32_t m_AppId = 0;

  public:
    void Initialize();

    const std::string& GetUsername() const
    {
        return m_Username;
    }

    const std::string& GetUserId() const
    {
        return m_UserId;
    }

    uint32_t GetAppId() const
    {
        return m_AppId;
    }

    void SetAppId(uint32_t new_value)
    {
        m_AppId = new_value;
    }
};

#endif
