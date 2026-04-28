#pragma once

namespace SceneJson
{
	inline std::string EscapeString(const std::string& value)
	{
		std::ostringstream oss;
		for (char ch : value)
		{
			switch (ch)
			{
			case '\\':
				oss << "\\\\";
				break;
			case '"':
				oss << "\\\"";
				break;
			case '\n':
				oss << "\\n";
				break;
			case '\r':
				oss << "\\r";
				break;
			case '\t':
				oss << "\\t";
				break;
			default:
				oss << ch;
				break;
			}
		}
		return oss.str();
	}

	inline bool FindValue(const std::string& json, const std::string& key, size_t& valueBegin)
	{
		const std::string keyToken = "\"" + key + "\"";
		const size_t keyPos = json.find(keyToken);
		if (keyPos == std::string::npos)
			return false;

		const size_t colonPos = json.find(':', keyPos + keyToken.length());
		if (colonPos == std::string::npos)
			return false;

		valueBegin = json.find_first_not_of(" \t\r\n", colonPos + 1);
		return valueBegin != std::string::npos;
	}

	inline bool ReadString(const std::string& json, const std::string& key, std::string& outValue)
	{
		size_t valueBegin = std::string::npos;
		if (!FindValue(json, key, valueBegin) || json[valueBegin] != '"')
			return false;

		outValue.clear();
		for (size_t i = valueBegin + 1; i < json.length(); i++)
		{
			const char ch = json[i];
			if (ch == '"')
				return true;

			if (ch == '\\' && i + 1 < json.length())
			{
				const char escaped = json[++i];
				switch (escaped)
				{
				case 'n':
					outValue.push_back('\n');
					break;
				case 'r':
					outValue.push_back('\r');
					break;
				case 't':
					outValue.push_back('\t');
					break;
				default:
					outValue.push_back(escaped);
					break;
				}
				continue;
			}

			outValue.push_back(ch);
		}
		return false;
	}

	inline bool ReadBool(const std::string& json, const std::string& key, bool& outValue)
	{
		size_t valueBegin = std::string::npos;
		if (!FindValue(json, key, valueBegin))
			return false;

		if (json.compare(valueBegin, 4, "true") == 0)
		{
			outValue = true;
			return true;
		}
		if (json.compare(valueBegin, 5, "false") == 0)
		{
			outValue = false;
			return true;
		}
		return false;
	}

	inline bool ReadInt(const std::string& json, const std::string& key, int& outValue)
	{
		size_t valueBegin = std::string::npos;
		if (!FindValue(json, key, valueBegin))
			return false;

		char* endPtr = nullptr;
		outValue = static_cast<int>(strtol(json.c_str() + valueBegin, &endPtr, 10));
		return endPtr != json.c_str() + valueBegin;
	}

	inline bool ReadDword(const std::string& json, const std::string& key, DWORD& outValue)
	{
		size_t valueBegin = std::string::npos;
		if (!FindValue(json, key, valueBegin))
			return false;

		char* endPtr = nullptr;
		outValue = static_cast<DWORD>(strtoul(json.c_str() + valueBegin, &endPtr, 10));
		return endPtr != json.c_str() + valueBegin;
	}

	inline bool ReadFloat(const std::string& json, const std::string& key, float& outValue)
	{
		size_t valueBegin = std::string::npos;
		if (!FindValue(json, key, valueBegin))
			return false;

		char* endPtr = nullptr;
		outValue = strtof(json.c_str() + valueBegin, &endPtr);
		return endPtr != json.c_str() + valueBegin;
	}

	inline bool ExtractObject(const std::string& json, const std::string& key, std::string& outObject)
	{
		size_t valueBegin = std::string::npos;
		if (!FindValue(json, key, valueBegin) || json[valueBegin] != '{')
			return false;

		bool inString = false;
		bool isEscaped = false;
		int depth = 0;
		for (size_t i = valueBegin; i < json.length(); i++)
		{
			const char ch = json[i];
			if (inString)
			{
				if (isEscaped)
					isEscaped = false;
				else if (ch == '\\')
					isEscaped = true;
				else if (ch == '"')
					inString = false;
				continue;
			}

			if (ch == '"')
			{
				inString = true;
				continue;
			}
			if (ch == '{')
				depth++;
			else if (ch == '}')
			{
				depth--;
				if (depth == 0)
				{
					outObject = json.substr(valueBegin, i - valueBegin + 1);
					return true;
				}
			}
		}
		return false;
	}

	inline bool ExtractArray(const std::string& json, const std::string& key, std::string& outArray)
	{
		size_t valueBegin = std::string::npos;
		if (!FindValue(json, key, valueBegin) || json[valueBegin] != '[')
			return false;

		bool inString = false;
		bool isEscaped = false;
		int depth = 0;
		for (size_t i = valueBegin; i < json.length(); i++)
		{
			const char ch = json[i];
			if (inString)
			{
				if (isEscaped)
					isEscaped = false;
				else if (ch == '\\')
					isEscaped = true;
				else if (ch == '"')
					inString = false;
				continue;
			}

			if (ch == '"')
			{
				inString = true;
				continue;
			}
			if (ch == '[')
				depth++;
			else if (ch == ']')
			{
				depth--;
				if (depth == 0)
				{
					outArray = json.substr(valueBegin, i - valueBegin + 1);
					return true;
				}
			}
		}
		return false;
	}

	inline bool ExtractObjectsFromArray(const std::string& arrayJson, std::vector<std::string>& outObjects)
	{
		bool inString = false;
		bool isEscaped = false;
		int depth = 0;
		size_t objectBegin = std::string::npos;

		for (size_t i = 0; i < arrayJson.length(); i++)
		{
			const char ch = arrayJson[i];
			if (inString)
			{
				if (isEscaped)
					isEscaped = false;
				else if (ch == '\\')
					isEscaped = true;
				else if (ch == '"')
					inString = false;
				continue;
			}

			if (ch == '"')
			{
				inString = true;
				continue;
			}
			if (ch == '{')
			{
				if (depth == 0)
					objectBegin = i;
				depth++;
			}
			else if (ch == '}')
			{
				depth--;
				if (depth == 0 && objectBegin != std::string::npos)
				{
					outObjects.push_back(arrayJson.substr(objectBegin, i - objectBegin + 1));
					objectBegin = std::string::npos;
				}
				else if (depth < 0)
				{
					return false;
				}
			}
		}
		return depth == 0 && !inString;
	}

	inline std::string WriteVector2(const char* name, const D3DXVECTOR2* value)
	{
		if (!value)
			return std::string();

		std::ostringstream oss;
		oss << "\"" << name << "\": { \"x\": " << value->x << ", \"y\": " << value->y << " }";
		return oss.str();
	}

	inline std::string WriteVector3(const char* name, const D3DXVECTOR3* value)
	{
		if (!value)
			return std::string();

		std::ostringstream oss;
		oss << "\"" << name << "\": { \"x\": " << value->x << ", \"y\": " << value->y << ", \"z\": " << value->z << " }";
		return oss.str();
	}

	inline bool ReadVector2(const std::string& json, const std::string& key, D3DXVECTOR2* outValue)
	{
		if (!outValue)
			return false;

		std::string objectJson;
		if (!ExtractObject(json, key, objectJson))
			return false;

		return ReadFloat(objectJson, "x", outValue->x)
			&& ReadFloat(objectJson, "y", outValue->y);
	}

	inline bool ReadVector3(const std::string& json, const std::string& key, D3DXVECTOR3* outValue)
	{
		if (!outValue)
			return false;

		std::string objectJson;
		if (!ExtractObject(json, key, objectJson))
			return false;

		return ReadFloat(objectJson, "x", outValue->x)
			&& ReadFloat(objectJson, "y", outValue->y)
			&& ReadFloat(objectJson, "z", outValue->z);
	}
}
