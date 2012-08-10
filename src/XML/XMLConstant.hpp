/* 
 * File:   XMLConstant.hpp
 * Author: zhang30
 *
 * Created on December 1, 2011, 12:09 PM
 */

#ifndef XMLCONSTANT_HPP
#define	XMLCONSTANT_HPP

namespace LBIND{
// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
// Including the basic of this table, which determines the #bytes in the
// sequence from the lead byte. 1 placed for invalid sequences --
// although the result will be junk, pass it through as much as possible.
// Beware of the non-characters in UTF-8:	
//				ef bb bf (Microsoft "lead bytes")
//				ef bf be
//				ef bf bf 
const unsigned char XML_UTF_LEAD_0 = 0xefU;
const unsigned char XML_UTF_LEAD_1 = 0xbbU;
const unsigned char XML_UTF_LEAD_2 = 0xbfU;

}//namespace LBIND
#endif	/* XMLCONSTANT_HPP */

