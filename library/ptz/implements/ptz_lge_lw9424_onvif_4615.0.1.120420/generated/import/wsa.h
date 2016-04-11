/* D:/intelli-vms/ws4d_gsoap/build/WS/wsa.h
   Generated by wsdl2h 1.2.13 from D:/intelli-vms/ws4d_gsoap/WS/wsa.xsd and D:/intelli-vms/ws4d_gsoap/build/WS/DPWS-typemap.dat
   2011-11-08 07:23:44 GMT
   gSOAP XML Web services tools.
   Copyright (C) 2001-2009 Robert van Engelen, Genivia Inc. All Rights Reserved.
   Part of this software is released under one of the following licenses:
   GPL or Genivia's license for commercial use.
*/

/* NOTE:

 - Compile this file with soapcpp2 to complete the code generation process.
 - Use soapcpp2 option -I to specify paths for #import
   To build with STL, 'stlvector.h' is imported from 'import' dir in package.
 - Use wsdl2h options -c and -s to generate pure C code or C++ code without STL.
 - Use 'typemap.dat' to control namespace bindings and type mappings.
   It is strongly recommended to customize the names of the namespace prefixes
   generated by wsdl2h. To do so, modify the prefix bindings in the Namespaces
   section below and add the modified lines to 'typemap.dat' to rerun wsdl2h.
 - Use Doxygen (www.doxygen.org) to browse this file.
 - Use wsdl2h option -l to view the software license terms.

   DO NOT include this file directly into your project.
   Include only the soapcpp2-generated headers and source code files.
*/


/******************************************************************************\
 *                                                                            *
 * http://schemas.xmlsoap.org/ws/2004/08/addressing                           *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Import                                                                     *
 *                                                                            *
\******************************************************************************/

#import "xsd.h"	// import primitive XSD types.

/******************************************************************************\
 *                                                                            *
 * Schema Namespaces                                                          *
 *                                                                            *
\******************************************************************************/


/* NOTE:

It is strongly recommended to customize the names of the namespace prefixes
generated by wsdl2h. To do so, modify the prefix bindings below and add the
modified lines to typemap.dat to rerun wsdl2h:

wsa = "http://schemas.xmlsoap.org/ws/2004/08/addressing"

*/

//gsoap wsa   schema namespace:	http://schemas.xmlsoap.org/ws/2004/08/addressing
//gsoap wsa   schema elementForm:	qualified
//gsoap wsa   schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Schema Types                                                               *
 *                                                                            *
\******************************************************************************/


//  xsd.h: typemap override of type xsd__NCName with char* /*NCName*/

//  xsd.h: typemap override of type xsd__QName with _QName

//  xsd.h: typemap override of type xsd__anyURI with char* /*URI*/

//  xsd.h: typemap override of type xsd__nonNegativeInteger with unsigned int


/// Typedef synonym for struct wsa__EndpointReferenceType.
typedef struct wsa__EndpointReferenceType wsa__EndpointReferenceType;

/// Typedef synonym for struct wsa__ReferencePropertiesType.
typedef struct wsa__ReferencePropertiesType wsa__ReferencePropertiesType;

/// Imported complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReferenceParametersType from typemap D:/intelli-vms/ws4d_gsoap/build/WS/DPWS-typemap.dat.
typedef struct wsa__ReferenceParametersType
{
    int                                  __sizepar                     ;
    struct wsa_ReferenceParameterAny
    {
        int                              __type                        ;
        void                            *__any                         ;
    }
                                   *__par                      ;
} wsa__ReferenceParametersType;

/// Typedef synonym for struct wsa__ServiceNameType.
typedef struct wsa__ServiceNameType wsa__ServiceNameType;

/// Typedef synonym for struct wsa__Relationship.
typedef struct wsa__Relationship wsa__Relationship;

/// Imported complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReplyAfterType from typemap D:/intelli-vms/ws4d_gsoap/build/WS/DPWS-typemap.dat.
// complexType definition intentionally left blank.

/// Imported complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedQName from typemap D:/intelli-vms/ws4d_gsoap/build/WS/DPWS-typemap.dat.
// complexType definition intentionally left blank.

/// Imported complexType "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI from typemap D:/intelli-vms/ws4d_gsoap/build/WS/DPWS-typemap.dat.
// complexType definition intentionally left blank.

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":RelationshipTypeValues is a simpleType restriction of xs:QName.
/// Note: enum values are prefixed with 'wsa__RelationshipTypeValues' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum wsa__RelationshipTypeValues
{
	wsa__RelationshipTypeValues__wsa__Reply,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":Reply"
};
/// Typedef synonym for enum wsa__RelationshipTypeValues.
typedef enum wsa__RelationshipTypeValues wsa__RelationshipTypeValues;

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":FaultSubcodeValues is a simpleType restriction of xs:QName.
/// Note: enum values are prefixed with 'wsa__FaultSubcodeValues' to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum wsa__FaultSubcodeValues
{
	wsa__FaultSubcodeValues__wsa__InvalidMessageInformationHeader,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":InvalidMessageInformationHeader"
	wsa__FaultSubcodeValues__wsa__MessageInformationHeaderRequired,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":MessageInformationHeaderRequired"
	wsa__FaultSubcodeValues__wsa__DestinationUnreachable,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":DestinationUnreachable"
	wsa__FaultSubcodeValues__wsa__ActionNotSupported,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":ActionNotSupported"
	wsa__FaultSubcodeValues__wsa__EndpointUnavailable,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointUnavailable"
};
/// Typedef synonym for enum wsa__FaultSubcodeValues.
typedef enum wsa__FaultSubcodeValues wsa__FaultSubcodeValues;

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType is a complexType.
struct wsa__EndpointReferenceType
{
/// Element Address of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI.
    char*                                Address                        1;	///< Required element.
/// Element ReferenceProperties of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReferencePropertiesType.
    struct wsa__ReferencePropertiesType*  ReferenceProperties            0;	///< Optional element.
/// Element ReferenceParameters of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReferenceParametersType.
    wsa__ReferenceParametersType*        ReferenceParameters            0;	///< Optional element.
/// Element PortType of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedQName.
    _QName*                              PortType                       0;	///< Optional element.
/// Element ServiceName of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":ServiceNameType.
    struct wsa__ServiceNameType*         ServiceName                    0;	///< Optional element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// Size of the array of XML or DOM nodes is 0..unbounded
    int                                  __size                        ;
    _XML                                *__any                         0;	///< Catch any element content in XML string.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
   @_XML                                 __anyAttribute                ;	///< A placeholder that has no effect: please see comment.
};

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReferencePropertiesType is a complexType.
struct wsa__ReferencePropertiesType
{
/// TODO: <any namespace="##any" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// Size of the array of XML or DOM nodes is 0..unbounded
    int                                  __size                        ;
    _XML                                *__any                         0;	///< Catch any element content in XML string.
};

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":ServiceNameType is a complexType with simpleContent.
struct wsa__ServiceNameType
{
/// __item wraps 'xs:QName' simpleContent.
    _QName                               __item                        ;
/// Attribute PortName of type xs:NCName.
   @char* /*NCName*/                     PortName                       0;	///< Optional attribute.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
   @_XML                                 __anyAttribute                ;	///< A placeholder that has no effect: please see comment.
};

/// "http://schemas.xmlsoap.org/ws/2004/08/addressing":Relationship is a complexType with simpleContent.
struct wsa__Relationship
{
/// __item wraps 'xs:anyURI' simpleContent.
    char* /*URI*/                        __item                        ;
/// Attribute RelationshipType of type xs:QName.
   @_QName                               RelationshipType               0;	///< Optional attribute.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
   @_XML                                 __anyAttribute                ;	///< A placeholder that has no effect: please see comment.
};

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReference of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType.
typedef struct wsa__EndpointReferenceType _wsa__EndpointReference;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":MessageID of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI.
typedef char* _wsa__MessageID;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":RelatesTo of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":Relationship.
typedef struct wsa__Relationship _wsa__RelatesTo;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":To of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI.
typedef char* _wsa__To;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":Action of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":AttributedURI.
typedef char* _wsa__Action;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":From of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType.
typedef struct wsa__EndpointReferenceType _wsa__From;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReplyTo of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType.
typedef struct wsa__EndpointReferenceType _wsa__ReplyTo;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":FaultTo of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReferenceType.
typedef struct wsa__EndpointReferenceType _wsa__FaultTo;

/// Element "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReplyAfter of type "http://schemas.xmlsoap.org/ws/2004/08/addressing":ReplyAfterType.
typedef unsigned int _wsa__ReplyAfter;

/// Attribute "http://schemas.xmlsoap.org/ws/2004/08/addressing":Action of simpleType xs:anyURI.
// '_wsa__Action' attribute definition intentionally left blank.

/* End of D:/intelli-vms/ws4d_gsoap/build/WS/wsa.h */

