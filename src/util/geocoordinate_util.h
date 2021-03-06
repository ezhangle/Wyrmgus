//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
//      (c) Copyright 2020 by Andrettin
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

#pragma once

namespace stratagus::geocoordinate {

static constexpr int longitude_size = 360;
static constexpr int latitude_size = 180;
static constexpr int min_longitude = longitude_size / 2 * -1;
static constexpr int max_longitude = longitude_size / 2;
static constexpr int min_latitude = latitude_size / 2 * -1;
static constexpr int max_latitude = latitude_size / 2;
static const QGeoCoordinate min_geocoordinate(geocoordinate::min_latitude, geocoordinate::min_longitude);
static const QGeoCoordinate max_geocoordinate(geocoordinate::max_latitude, geocoordinate::max_longitude);

inline double longitude_to_unsigned_longitude(const double longitude)
{
	return longitude + (geocoordinate::longitude_size / 2);
}

inline double latitude_to_unsigned_latitude(const double latitude)
{
	return latitude * -1 + (geocoordinate::latitude_size / 2);
}

inline QPointF to_unsigned_geocoordinate(const QGeoCoordinate &geocoordinate)
{
	//converts a geocoordinate into a (floating) coordinate point, having x values from 0 to 360, and y values from 0 to 180 (top to bottom, contrary to geocoordinates, which work south to north)
	const double x = geocoordinate::longitude_to_unsigned_longitude(geocoordinate.longitude());
	const double y = geocoordinate::latitude_to_unsigned_latitude(geocoordinate.latitude());
	return QPointF(x, y);
}

inline QGeoCoordinate from_unsigned_geocoordinate(const QPointF &unsigned_geocoordinate)
{
	const double lon = unsigned_geocoordinate.x() - (geocoordinate::longitude_size / 2);
	const double lat = (unsigned_geocoordinate.y() - (geocoordinate::latitude_size / 2)) * -1;
	return QGeoCoordinate(lat, lon);
}

inline int unsigned_longitude_to_x(const double longitude, const double lon_per_pixel)
{
	const double x = longitude / lon_per_pixel;
	return static_cast<int>(std::round(x));
}

inline int unsigned_latitude_to_y(const double latitude, const double lat_per_pixel)
{
	const double y = latitude / lat_per_pixel;
	return static_cast<int>(std::round(y));
}

inline int longitude_to_x(const double longitude, const double lon_per_pixel)
{
	const double x = geocoordinate::longitude_to_unsigned_longitude(longitude);
	return geocoordinate::unsigned_longitude_to_x(x, lon_per_pixel);
}

inline int latitude_to_y(const double latitude, const double lat_per_pixel)
{
	const double y = geocoordinate::latitude_to_unsigned_latitude(latitude);
	return geocoordinate::unsigned_latitude_to_y(y, lat_per_pixel);
}

inline double longitude_per_pixel(const double longitude_size, const QSize &size)
{
	return longitude_size / static_cast<double>(size.width());
}

inline double latitude_per_pixel(const double latitude_size, const QSize &size)
{
	return latitude_size / static_cast<double>(size.height());
}

inline QPoint to_point(const QGeoCoordinate &geocoordinate, const double lon_per_pixel, const double lat_per_pixel)
{
	const int x = geocoordinate::longitude_to_x(geocoordinate.longitude(), lon_per_pixel);
	const int y = geocoordinate::latitude_to_y(geocoordinate.latitude(), lat_per_pixel);
	return QPoint(x, y);
}

inline QPoint to_point(const QGeoCoordinate &geocoordinate, const QGeoRectangle &georectangle, const QSize &area_size)
{
	const double lon_per_pixel = geocoordinate::longitude_per_pixel(georectangle.width(), area_size);
	const double lat_per_pixel = geocoordinate::latitude_per_pixel(georectangle.height(), area_size);
	const QPoint geocoordinate_offset = geocoordinate::to_point(georectangle.topLeft(), lon_per_pixel, lat_per_pixel);
	return geocoordinate::to_point(geocoordinate, lon_per_pixel, lat_per_pixel) - geocoordinate_offset;
}

}
