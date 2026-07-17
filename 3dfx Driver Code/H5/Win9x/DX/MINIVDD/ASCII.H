/*
 * Name:        ascii.h
 *
 * Description: This module contains all the ASCII equate definitions to be
 *              used by programs.
 *
 * Version:     1.00
 *
 * Date:        August 1, 1998
 *
 * Author:      Randy Spurlock
 *
 **********************
 *
 * Changes:     Date            Version Description
 *
 *              08/01/1998      1.00    Original version
 *              09/25/1998      1.01    Converted to C from ascii.asm
 * 
 */
#ifndef __ASCII_H
#define __ASCII_H
#if _MSC_VER >= 1000
#pragma once
#endif





#define         ASCII_NULL              0x00
#define         ASCII_SOH               0x01
#define         ASCII_STX               0x02
#define         ASCII_ETX               0x03
#define         ASCII_EOT               0x04
#define         ASCII_ENQ               0x05
#define         ASCII_ACK               0x06
#define         ASCII_BELL              0x07
#define         ASCII_BS                0x08
#define         ASCII_HT                0x09
#define         ASCII_LF                0x0a
#define         ASCII_VT                0x0b
#define         ASCII_FF                0x0c
#define         ASCII_CR                0x0d
#define         ASCII_SO                0x0e
#define         ASCII_SI                0x0f
#define         ASCII_SLE               0x10
#define         ASCII_CS1               0x11
#define         ASCII_DC2               0x12
#define         ASCII_DC3               0x13
#define         ASCII_DC4               0x14
#define         ASCII_NAK               0x15
#define         ASCII_SYN               0x16
#define         ASCII_ETB               0x17
#define         ASCII_CAN               0x18
#define         ASCII_EM                0x19
#define         ASCII_EOF               0x1a
#define         ASCII_ESCAPE            0x1b

#define         ASCII_SPACE             ' '
#define         ASCII_DOLLAR            '$'
#define         ASCII_PERCENT           '%'
#define         ASCII_ASTERICK          '*'
#define         ASCII_PLUS              '+'
#define         ASCII_COMMA             ','
#define         ASCII_MINUS             '-'
#define         ASCII_PERIOD            '.'
#define         ASCII_SLASH             '/'
#define         ASCII_ZERO              '0'
#define         ASCII_COLON             ':'
#define         ASCII_SEMI_COLON        ';'
#define         ASCII_EQUAL             '='
#define         ASCII_AT_SIGN           '@'
#define         ASCII_BACK_SLASH        '\\'

#define         ASCII_SPC               0xff





#endif

