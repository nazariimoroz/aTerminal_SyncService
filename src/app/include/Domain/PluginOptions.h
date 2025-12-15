#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#include <stduuid/uuid.h>

namespace Domain
{
    class PluginOptions
    {
    public:
        int getId() const { return _id; }
        void setId(const int id) { _id = id; }

        int getUserId() const { return _userId; }
        void setUserId(const int userId) { _userId = userId; }

        const std::string& getPluginName() const { return _pluginName; }
        void setPluginName(std::string pluginName) { _pluginName = std::move(pluginName); }

        bool isEnabled() const { return _isEnabled; }
        void setEnabled(const bool isEnabled) { _isEnabled = isEnabled; }

        bool isFavorite() const { return _isFavorite; }
        void setFavorite(const bool isFavorite) { _isFavorite = isFavorite; }

        const std::unordered_map<std::string, std::string>& getFields() const { return _fields; }
        void setFields(std::unordered_map<std::string, std::string> fields) { _fields = std::move(fields); }

    protected:
        int _id = 0;
        int _userId = 0;
        std::string _pluginName;
        bool _isEnabled = false;
        bool _isFavorite = false;
        std::unordered_map<std::string, std::string> _fields;
    };
} // namespace Domain
