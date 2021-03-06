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
//      (c) Copyright 2019-2020 by Andrettin
//
//      Permission is hereby granted, free of charge, to any person obtaining a
//      copy of this software and associated documentation files (the
//      "Software"), to deal in the Software without restriction, including
//      without limitation the rights to use, copy, modify, merge, publish,
//      distribute, sublicense, and/or sell copies of the Software, and to
//      permit persons to whom the Software is furnished to do so, subject to
//      the following conditions:
//
//      The above copyright notice and this permission notice shall be included
//      in all copies or substantial portions of the Software.
//
//      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//      OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//      MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//      CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//      TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "database/sml_data_visitor.h"
#include "database/sml_element_visitor.h"
#include "database/sml_property.h"
#include "database/sml_property_visitor.h"

namespace stratagus {

class sml_parser;

//stratagus markup language data
class sml_data
{
public:
	template <typename point_type>
	static sml_data from_point(const point_type &point, const std::string &tag = std::string())
	{
		sml_data point_data(tag);
		point_data.add_value(std::to_string(point.x()));
		point_data.add_value(std::to_string(point.y()));
		return point_data;
	}

	sml_data(std::string &&tag = std::string());

	sml_data(std::string &&tag, const sml_operator scope_operator)
		: tag(std::move(tag)), scope_operator(scope_operator)
	{
	}

	sml_data(const std::string &tag) : sml_data(std::string(tag))
	{
	}

	sml_data(const std::string &tag, const sml_operator scope_operator)
		: sml_data(std::string(tag), scope_operator)
	{
	}

	const std::string &get_tag() const
	{
		return this->tag;
	}

	sml_operator get_operator() const
	{
		return this->scope_operator;
	}

	const sml_data *get_parent() const
	{
		return this->parent;
	}

	bool has_children() const
	{
		for (const auto &element : this->get_elements()) {
			if (std::holds_alternative<sml_data>(element)) {
				return true;
			}
		}

		return false;
	}

	const sml_data &get_child(const std::string &tag) const
	{
		for (const auto &element : this->get_elements()) {
			if (!std::holds_alternative<sml_data>(element)) {
				continue;
			}

			const sml_data &child = std::get<sml_data>(element);
			if (child.get_tag() == tag) {
				return child;
			}
		}

		throw std::runtime_error("No child with tag \"" + tag + "\" found for SML data.");
	}

	bool has_child(const std::string &tag) const
	{
		for (const auto &element : this->get_elements()) {
			if (!std::holds_alternative<sml_data>(element)) {
				continue;
			}

			if (std::get<sml_data>(element).get_tag() == tag) {
				return true;
			}
		}

		return false;
	}

	sml_data &add_child()
	{
		this->elements.push_back(sml_data());
		return std::get<sml_data>(this->elements.back());
	}

	void add_child(sml_data &&child)
	{
		this->elements.emplace_back(std::move(child));
	}

	sml_data &add_child(std::string &&tag, const sml_operator sml_operator)
	{
		this->elements.push_back(sml_data(std::move(tag), sml_operator));
		return std::get<sml_data>(this->elements.back());
	}

	template <typename function_type>
	void for_each_child(const function_type &function) const
	{
		const sml_data_visitor visitor(function);
		for (const auto &element : this->get_elements()) {
			std::visit(visitor, element);
		}
	}

	std::vector<const sml_property *> try_get_properties(const std::string &key) const
	{
		std::vector<const sml_property *> properties;

		this->for_each_property([&](const sml_property &property) {
			if (property.get_key() == key) {
				properties.push_back(&property);
			}
		});

		return properties;
	}

	const std::string &get_property_value(const std::string &key) const
	{
		for (const auto &element : this->get_elements()) {
			if (!std::holds_alternative<sml_property>(element)) {
				continue;
			}

			const sml_property &property = std::get<sml_property>(element);
			if (property.get_key() == key) {
				return property.get_value();
			}
		}

		throw std::runtime_error("No property with key \"" + key + "\" found for SML data.");
	}

	void add_property(const std::string &key, const std::string &value);
	void add_property(std::string &&key, const sml_operator sml_operator, std::string &&value);

	template <typename function_type>
	void for_each_property(const function_type &function) const
	{
		const sml_property_visitor visitor(function);
		for (const auto &element : this->get_elements()) {
			std::visit(visitor, element);
		}
	}

	const std::vector<std::string> &get_values() const
	{
		return this->values;
	}

	void add_value(const std::string &value)
	{
		this->values.push_back(value);
	}

	void add_value(std::string &&value)
	{
		this->values.push_back(std::move(value));
	}

	bool is_empty() const
	{
		return this->get_elements().empty() && this->get_values().empty();
	}

	const std::vector<std::variant<sml_property, sml_data>> &get_elements() const
	{
		return this->elements;
	}

	template <typename property_function_type, typename data_function_type>
	void for_each_element(const property_function_type &property_function, const data_function_type &data_function) const
	{
		const sml_element_visitor visitor(property_function, data_function);
		for (const auto &element : this->get_elements()) {
			std::visit(visitor, element);
		}
	}

	QColor to_color() const
	{
		if (this->get_values().size() != 3) {
			throw std::runtime_error("Color scopes need to contain exactly three values.");
		}

		const int red = std::stoi(this->values.at(0));
		const int green = std::stoi(this->values.at(1));
		const int blue = std::stoi(this->values.at(2));

		return QColor(red, green, blue);
	}

	QPoint to_point() const
	{
		if (this->get_values().size() != 2) {
			throw std::runtime_error("Point scopes need to contain exactly two values.");
		}

		const int x = std::stoi(this->get_values()[0]);
		const int y = std::stoi(this->get_values()[1]);
		return QPoint(x, y);
	}

	QPointF to_pointf() const
	{
		if (this->get_values().size() != 2) {
			throw std::runtime_error("Point scopes need to contain exactly two values.");
		}

		const double x = std::stod(this->get_values()[0]);
		const double y = std::stod(this->get_values()[1]);
		return QPointF(x, y);
	}

	QSize to_size() const
	{
		if (this->get_values().size() != 2) {
			throw std::runtime_error("Size scopes need to contain exactly two values.");
		}

		const int width = std::stoi(this->get_values()[0]);
		const int height = std::stoi(this->get_values()[1]);
		return QSize(width, height);
	}

	QGeoCoordinate to_geocoordinate() const
	{
		if (this->get_values().size() != 2) {
			throw std::runtime_error("Geocoordinate scopes need to contain exactly two values.");
		}

		const double longitude = std::stod(this->get_values()[0]);
		const double latitude = std::stod(this->get_values()[1]);
		return QGeoCoordinate(latitude, longitude);
	}

	void print_to_dir(const std::filesystem::path &directory) const
	{
		std::filesystem::path filepath(directory / (this->get_tag() + ".txt"));
		std::ofstream ofstream(filepath);
		this->print_components(ofstream);
	}

	void print(std::ofstream &ofstream, const size_t indentation, const bool new_line) const;

	void print_components(std::ofstream &ofstream, const size_t indentation = 0) const
	{
		if (!this->get_values().empty()) {
			if (this->is_minor()) {
				ofstream << " ";
			} else {
				ofstream << std::string(indentation, '\t');
			}
		}
		for (const std::string &value : this->get_values()) {
			ofstream << value << " ";
		}
		if (!this->get_values().empty()) {
			if (!this->is_minor()) {
				ofstream << "\n";
			}
		}

		this->for_each_property([&](const sml_property &property) {
			property.print(ofstream, indentation);
		});

		bool new_line = true;
		this->for_each_child([&](const sml_data &child_data) {
			child_data.print(ofstream, indentation, new_line);
			if (new_line && child_data.is_minor()) {
				new_line = false;
			}
		});

		//if the last child data was minor and did not print a new line, print one now
		if (!new_line) {
			ofstream << "\n";
		}
	}

private:
	bool is_minor() const
	{
		//get whether this is minor SML data, e.g. just containing a few simple values
		return this->get_tag().empty() && this->get_elements().empty() && this->get_values().size() < 5;
	}

private:
	std::string tag;
	sml_operator scope_operator;
	sml_data *parent = nullptr;
	std::vector<std::string> values; //values directly attached to the SML data scope, used for e.g. name arrays
	std::vector<std::variant<sml_property, sml_data>> elements;

	friend sml_parser;
};

}
