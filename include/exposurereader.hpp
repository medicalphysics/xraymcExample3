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

#include <string>
#include <vector>

#pragma once

struct Exposure {
    double primaryAngle = 0;
    double secondaryAngle = 0;
    double fieldSize1D = 1;
    double KAP = 1;
    double kV = 60;
    double SID = 100;
    double mmAl = 4.0;
    double mmCu = 0.1;
    double tableHeight = 0;
    double tableLateral = 0;
    double tableLongitudinal = 0;
};

class ExposureReader {
public:
    ExposureReader() = default;
    ExposureReader(const std::string& path)
        : m_path(path)
    {
    }
    std::vector<Exposure> operator()() const { return readData(m_path); };
    static std::vector<Exposure> readData(const std::string& path);

protected:
    std::string m_path;
};