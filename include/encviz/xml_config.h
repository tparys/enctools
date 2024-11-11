#pragma once
#include <string>
#include <vector>
#include <tinyxml2.h>

namespace encviz
{

/**
 * Query node text with checking
 *
 * \param[in] node Element with text
 * \return Tag text
 */
const char *xml_text(tinyxml2::XMLElement *node);

/**
 * Query all child nodes of type
 *
 * \param[in] root XML root node
 * \param[in] name XML tag name
 * \return All matching nodes
 */
std::vector<tinyxml2::XMLElement*> xml_query_all(tinyxml2::XMLElement *root,
                                                 const char *name);

/**
 * Query unique child node
 *
 * \param[in] root XML root node
 * \param[in] name XML tag name
 * \return Returned node
 */
tinyxml2::XMLElement *xml_query(tinyxml2::XMLElement *root,
                                const char *name);

}; // ~namespace encviz
