/*This file is part of xraymcCarm.

xraymcCarm is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

xraymcCarm is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with xraymcCarm. If not, see < https://www.gnu.org/licenses/>.

Copyright 2025 Erlend Andersen
*/

#include <algorithm>
#include <array>
#include <charconv>
#include <execution>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>

#include "exposurereader.hpp"

bool stringHasKeys(const std::string& str, const std::string& key)
{
    return str.find(key) != std::string::npos;
}

template <typename... Types>
bool stringHasKeys(const std::string& str, const std::string& key, Types... keys)
{
    return stringHasKeys(str, key) && stringHasKeys(str, keys...);
}

std::optional<double> toDouble(const std::string& word)
{

    double result = -1;

    auto start = word.data();
    while (*start == ' ' && start != word.data() + word.size()) {
        start++;
    }

    auto [ptr, ec] = std::from_chars(start, word.data() + word.size(), result);

    if (ec == std::errc())
        return result;
    else
        return std::nullopt;
}

std::vector<std::string> stringSplit(const std::string& str, const std::string& delim)
{
    std::vector<std::string> result;
    size_t start = 0;

    for (size_t found = str.find(delim); found != std::string::npos; found = str.find(delim, start)) {
        result.emplace_back(str.begin() + start, str.begin() + found);
        start = found + delim.size();
    }
    if (start != str.size())
        result.emplace_back(str.begin() + start, str.end());
    return result;
}
void stringToLower(std::string& str)
{
    std::transform(std::execution::par, str.cbegin(), str.cend(), str.begin(), [](unsigned char c) { return std::tolower(c); });
}

std::vector<std::string> readASCIILines(const std::string& path)
{
    std::vector<std::string> lines;
    std::ifstream t(path);
    std::stringstream buffer;
    if (t.is_open()) {
        buffer << t.rdbuf();
        t.close();
    } else {
        return lines;
    }
    for (std::string line; std::getline(buffer, line);)
        lines.emplace_back(line);
    return lines;
}

std::vector<Exposure> ExposureReader::readData(const std::string& path)
{
    auto lines = readASCIILines(path);
    for (auto& line : lines)
        stringToLower(line);

    std::vector<Exposure> results;
    if (lines.size() <= 1)
        return results;

    const auto header = stringSplit(lines[0], ",");
    const std::array<std::string, 11> keys = { "al mm", "cu mm", "distance source to detector", "collimated field area", "tube voltage", "dap", "positioner primary", "positioner secondary", "height", "lateral", "longitudinal" };
    std::map<std::string, int> wIdx;
    for (int i = 0; i < header.size(); ++i)
        for (const auto& k : keys)
            if (stringHasKeys(header[i], k))
                wIdx[k] = i;
    if (wIdx.size() >= 10) {
        for (std::size_t lineIdx = 1; lineIdx < lines.size(); ++lineIdx) {

            bool valid = true;
            const auto words = stringSplit(lines[lineIdx], ",");
            if (words.size() >= wIdx.size()) {
                std::array<std::optional<double>, 11> vals;
                for (std::size_t i = 0; i < keys.size(); ++i) {
                    const auto arrIdx = wIdx.at(keys[i]);
                    vals[i] = toDouble(words[arrIdx]);
                }
                valid = valid && std::all_of(vals.cbegin(), vals.cend(), [](const auto& v) { return v.has_value(); });
                if (valid) {
                    Exposure exp;
                    exp.mmAl = vals[0].value();
                    exp.mmCu = vals[1].value();
                    exp.SID = vals[2].value();
                    exp.fieldSize1D = vals[3].value();
                    exp.kV = vals[4].value();
                    exp.KAP = vals[5].value();
                    exp.primaryAngle = vals[6].value();
                    exp.secondaryAngle = vals[7].value();
                    exp.tableHeight = vals[8].value();
                    exp.tableLateral = vals[9].value();
                    exp.tableLongitudinal = vals[10].value();
                    results.push_back(exp);
                }
            }
        }
    }
    return results;
}
