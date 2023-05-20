#/* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  */
#
#/* Revised by Paul Mensonides (2002) */
#
#/* See http://www.boost.org for most recent version. */
#
#ifndef MSGPACK_PREPROCESSOR_CONTROL_DETAIL_WHILE_HPP
#define MSGPACK_PREPROCESSOR_CONTROL_DETAIL_WHILE_HPP
#
#include <msgpack/preprocessor/control/iif.hpp>
#include <msgpack/preprocessor/logical/bool.hpp>
#include <msgpack/preprocessor/tuple/eat.hpp>
#
#define MSGPACK_PP_WHILE_1(p, o, s) MSGPACK_PP_WHILE_1_C(MSGPACK_PP_BOOL(p(2, s)), p, o, s)
#define MSGPACK_PP_WHILE_2(p, o, s) MSGPACK_PP_WHILE_2_C(MSGPACK_PP_BOOL(p(3, s)), p, o, s)
#define MSGPACK_PP_WHILE_3(p, o, s) MSGPACK_PP_WHILE_3_C(MSGPACK_PP_BOOL(p(4, s)), p, o, s)
#define MSGPACK_PP_WHILE_4(p, o, s) MSGPACK_PP_WHILE_4_C(MSGPACK_PP_BOOL(p(5, s)), p, o, s)
#define MSGPACK_PP_WHILE_5(p, o, s) MSGPACK_PP_WHILE_5_C(MSGPACK_PP_BOOL(p(6, s)), p, o, s)
#define MSGPACK_PP_WHILE_6(p, o, s) MSGPACK_PP_WHILE_6_C(MSGPACK_PP_BOOL(p(7, s)), p, o, s)
#define MSGPACK_PP_WHILE_7(p, o, s) MSGPACK_PP_WHILE_7_C(MSGPACK_PP_BOOL(p(8, s)), p, o, s)
#define MSGPACK_PP_WHILE_8(p, o, s) MSGPACK_PP_WHILE_8_C(MSGPACK_PP_BOOL(p(9, s)), p, o, s)
#define MSGPACK_PP_WHILE_9(p, o, s) MSGPACK_PP_WHILE_9_C(MSGPACK_PP_BOOL(p(10, s)), p, o, s)
#define MSGPACK_PP_WHILE_10(p, o, s) MSGPACK_PP_WHILE_10_C(MSGPACK_PP_BOOL(p(11, s)), p, o, s)
#define MSGPACK_PP_WHILE_11(p, o, s) MSGPACK_PP_WHILE_11_C(MSGPACK_PP_BOOL(p(12, s)), p, o, s)
#define MSGPACK_PP_WHILE_12(p, o, s) MSGPACK_PP_WHILE_12_C(MSGPACK_PP_BOOL(p(13, s)), p, o, s)
#define MSGPACK_PP_WHILE_13(p, o, s) MSGPACK_PP_WHILE_13_C(MSGPACK_PP_BOOL(p(14, s)), p, o, s)
#define MSGPACK_PP_WHILE_14(p, o, s) MSGPACK_PP_WHILE_14_C(MSGPACK_PP_BOOL(p(15, s)), p, o, s)
#define MSGPACK_PP_WHILE_15(p, o, s) MSGPACK_PP_WHILE_15_C(MSGPACK_PP_BOOL(p(16, s)), p, o, s)
#define MSGPACK_PP_WHILE_16(p, o, s) MSGPACK_PP_WHILE_16_C(MSGPACK_PP_BOOL(p(17, s)), p, o, s)
#define MSGPACK_PP_WHILE_17(p, o, s) MSGPACK_PP_WHILE_17_C(MSGPACK_PP_BOOL(p(18, s)), p, o, s)
#define MSGPACK_PP_WHILE_18(p, o, s) MSGPACK_PP_WHILE_18_C(MSGPACK_PP_BOOL(p(19, s)), p, o, s)
#define MSGPACK_PP_WHILE_19(p, o, s) MSGPACK_PP_WHILE_19_C(MSGPACK_PP_BOOL(p(20, s)), p, o, s)
#define MSGPACK_PP_WHILE_20(p, o, s) MSGPACK_PP_WHILE_20_C(MSGPACK_PP_BOOL(p(21, s)), p, o, s)
#define MSGPACK_PP_WHILE_21(p, o, s) MSGPACK_PP_WHILE_21_C(MSGPACK_PP_BOOL(p(22, s)), p, o, s)
#define MSGPACK_PP_WHILE_22(p, o, s) MSGPACK_PP_WHILE_22_C(MSGPACK_PP_BOOL(p(23, s)), p, o, s)
#define MSGPACK_PP_WHILE_23(p, o, s) MSGPACK_PP_WHILE_23_C(MSGPACK_PP_BOOL(p(24, s)), p, o, s)
#define MSGPACK_PP_WHILE_24(p, o, s) MSGPACK_PP_WHILE_24_C(MSGPACK_PP_BOOL(p(25, s)), p, o, s)
#define MSGPACK_PP_WHILE_25(p, o, s) MSGPACK_PP_WHILE_25_C(MSGPACK_PP_BOOL(p(26, s)), p, o, s)
#define MSGPACK_PP_WHILE_26(p, o, s) MSGPACK_PP_WHILE_26_C(MSGPACK_PP_BOOL(p(27, s)), p, o, s)
#define MSGPACK_PP_WHILE_27(p, o, s) MSGPACK_PP_WHILE_27_C(MSGPACK_PP_BOOL(p(28, s)), p, o, s)
#define MSGPACK_PP_WHILE_28(p, o, s) MSGPACK_PP_WHILE_28_C(MSGPACK_PP_BOOL(p(29, s)), p, o, s)
#define MSGPACK_PP_WHILE_29(p, o, s) MSGPACK_PP_WHILE_29_C(MSGPACK_PP_BOOL(p(30, s)), p, o, s)
#define MSGPACK_PP_WHILE_30(p, o, s) MSGPACK_PP_WHILE_30_C(MSGPACK_PP_BOOL(p(31, s)), p, o, s)
#define MSGPACK_PP_WHILE_31(p, o, s) MSGPACK_PP_WHILE_31_C(MSGPACK_PP_BOOL(p(32, s)), p, o, s)
#define MSGPACK_PP_WHILE_32(p, o, s) MSGPACK_PP_WHILE_32_C(MSGPACK_PP_BOOL(p(33, s)), p, o, s)
#define MSGPACK_PP_WHILE_33(p, o, s) MSGPACK_PP_WHILE_33_C(MSGPACK_PP_BOOL(p(34, s)), p, o, s)
#define MSGPACK_PP_WHILE_34(p, o, s) MSGPACK_PP_WHILE_34_C(MSGPACK_PP_BOOL(p(35, s)), p, o, s)
#define MSGPACK_PP_WHILE_35(p, o, s) MSGPACK_PP_WHILE_35_C(MSGPACK_PP_BOOL(p(36, s)), p, o, s)
#define MSGPACK_PP_WHILE_36(p, o, s) MSGPACK_PP_WHILE_36_C(MSGPACK_PP_BOOL(p(37, s)), p, o, s)
#define MSGPACK_PP_WHILE_37(p, o, s) MSGPACK_PP_WHILE_37_C(MSGPACK_PP_BOOL(p(38, s)), p, o, s)
#define MSGPACK_PP_WHILE_38(p, o, s) MSGPACK_PP_WHILE_38_C(MSGPACK_PP_BOOL(p(39, s)), p, o, s)
#define MSGPACK_PP_WHILE_39(p, o, s) MSGPACK_PP_WHILE_39_C(MSGPACK_PP_BOOL(p(40, s)), p, o, s)
#define MSGPACK_PP_WHILE_40(p, o, s) MSGPACK_PP_WHILE_40_C(MSGPACK_PP_BOOL(p(41, s)), p, o, s)
#define MSGPACK_PP_WHILE_41(p, o, s) MSGPACK_PP_WHILE_41_C(MSGPACK_PP_BOOL(p(42, s)), p, o, s)
#define MSGPACK_PP_WHILE_42(p, o, s) MSGPACK_PP_WHILE_42_C(MSGPACK_PP_BOOL(p(43, s)), p, o, s)
#define MSGPACK_PP_WHILE_43(p, o, s) MSGPACK_PP_WHILE_43_C(MSGPACK_PP_BOOL(p(44, s)), p, o, s)
#define MSGPACK_PP_WHILE_44(p, o, s) MSGPACK_PP_WHILE_44_C(MSGPACK_PP_BOOL(p(45, s)), p, o, s)
#define MSGPACK_PP_WHILE_45(p, o, s) MSGPACK_PP_WHILE_45_C(MSGPACK_PP_BOOL(p(46, s)), p, o, s)
#define MSGPACK_PP_WHILE_46(p, o, s) MSGPACK_PP_WHILE_46_C(MSGPACK_PP_BOOL(p(47, s)), p, o, s)
#define MSGPACK_PP_WHILE_47(p, o, s) MSGPACK_PP_WHILE_47_C(MSGPACK_PP_BOOL(p(48, s)), p, o, s)
#define MSGPACK_PP_WHILE_48(p, o, s) MSGPACK_PP_WHILE_48_C(MSGPACK_PP_BOOL(p(49, s)), p, o, s)
#define MSGPACK_PP_WHILE_49(p, o, s) MSGPACK_PP_WHILE_49_C(MSGPACK_PP_BOOL(p(50, s)), p, o, s)
#define MSGPACK_PP_WHILE_50(p, o, s) MSGPACK_PP_WHILE_50_C(MSGPACK_PP_BOOL(p(51, s)), p, o, s)
#define MSGPACK_PP_WHILE_51(p, o, s) MSGPACK_PP_WHILE_51_C(MSGPACK_PP_BOOL(p(52, s)), p, o, s)
#define MSGPACK_PP_WHILE_52(p, o, s) MSGPACK_PP_WHILE_52_C(MSGPACK_PP_BOOL(p(53, s)), p, o, s)
#define MSGPACK_PP_WHILE_53(p, o, s) MSGPACK_PP_WHILE_53_C(MSGPACK_PP_BOOL(p(54, s)), p, o, s)
#define MSGPACK_PP_WHILE_54(p, o, s) MSGPACK_PP_WHILE_54_C(MSGPACK_PP_BOOL(p(55, s)), p, o, s)
#define MSGPACK_PP_WHILE_55(p, o, s) MSGPACK_PP_WHILE_55_C(MSGPACK_PP_BOOL(p(56, s)), p, o, s)
#define MSGPACK_PP_WHILE_56(p, o, s) MSGPACK_PP_WHILE_56_C(MSGPACK_PP_BOOL(p(57, s)), p, o, s)
#define MSGPACK_PP_WHILE_57(p, o, s) MSGPACK_PP_WHILE_57_C(MSGPACK_PP_BOOL(p(58, s)), p, o, s)
#define MSGPACK_PP_WHILE_58(p, o, s) MSGPACK_PP_WHILE_58_C(MSGPACK_PP_BOOL(p(59, s)), p, o, s)
#define MSGPACK_PP_WHILE_59(p, o, s) MSGPACK_PP_WHILE_59_C(MSGPACK_PP_BOOL(p(60, s)), p, o, s)
#define MSGPACK_PP_WHILE_60(p, o, s) MSGPACK_PP_WHILE_60_C(MSGPACK_PP_BOOL(p(61, s)), p, o, s)
#define MSGPACK_PP_WHILE_61(p, o, s) MSGPACK_PP_WHILE_61_C(MSGPACK_PP_BOOL(p(62, s)), p, o, s)
#define MSGPACK_PP_WHILE_62(p, o, s) MSGPACK_PP_WHILE_62_C(MSGPACK_PP_BOOL(p(63, s)), p, o, s)
#define MSGPACK_PP_WHILE_63(p, o, s) MSGPACK_PP_WHILE_63_C(MSGPACK_PP_BOOL(p(64, s)), p, o, s)
#define MSGPACK_PP_WHILE_64(p, o, s) MSGPACK_PP_WHILE_64_C(MSGPACK_PP_BOOL(p(65, s)), p, o, s)
#define MSGPACK_PP_WHILE_65(p, o, s) MSGPACK_PP_WHILE_65_C(MSGPACK_PP_BOOL(p(66, s)), p, o, s)
#define MSGPACK_PP_WHILE_66(p, o, s) MSGPACK_PP_WHILE_66_C(MSGPACK_PP_BOOL(p(67, s)), p, o, s)
#define MSGPACK_PP_WHILE_67(p, o, s) MSGPACK_PP_WHILE_67_C(MSGPACK_PP_BOOL(p(68, s)), p, o, s)
#define MSGPACK_PP_WHILE_68(p, o, s) MSGPACK_PP_WHILE_68_C(MSGPACK_PP_BOOL(p(69, s)), p, o, s)
#define MSGPACK_PP_WHILE_69(p, o, s) MSGPACK_PP_WHILE_69_C(MSGPACK_PP_BOOL(p(70, s)), p, o, s)
#define MSGPACK_PP_WHILE_70(p, o, s) MSGPACK_PP_WHILE_70_C(MSGPACK_PP_BOOL(p(71, s)), p, o, s)
#define MSGPACK_PP_WHILE_71(p, o, s) MSGPACK_PP_WHILE_71_C(MSGPACK_PP_BOOL(p(72, s)), p, o, s)
#define MSGPACK_PP_WHILE_72(p, o, s) MSGPACK_PP_WHILE_72_C(MSGPACK_PP_BOOL(p(73, s)), p, o, s)
#define MSGPACK_PP_WHILE_73(p, o, s) MSGPACK_PP_WHILE_73_C(MSGPACK_PP_BOOL(p(74, s)), p, o, s)
#define MSGPACK_PP_WHILE_74(p, o, s) MSGPACK_PP_WHILE_74_C(MSGPACK_PP_BOOL(p(75, s)), p, o, s)
#define MSGPACK_PP_WHILE_75(p, o, s) MSGPACK_PP_WHILE_75_C(MSGPACK_PP_BOOL(p(76, s)), p, o, s)
#define MSGPACK_PP_WHILE_76(p, o, s) MSGPACK_PP_WHILE_76_C(MSGPACK_PP_BOOL(p(77, s)), p, o, s)
#define MSGPACK_PP_WHILE_77(p, o, s) MSGPACK_PP_WHILE_77_C(MSGPACK_PP_BOOL(p(78, s)), p, o, s)
#define MSGPACK_PP_WHILE_78(p, o, s) MSGPACK_PP_WHILE_78_C(MSGPACK_PP_BOOL(p(79, s)), p, o, s)
#define MSGPACK_PP_WHILE_79(p, o, s) MSGPACK_PP_WHILE_79_C(MSGPACK_PP_BOOL(p(80, s)), p, o, s)
#define MSGPACK_PP_WHILE_80(p, o, s) MSGPACK_PP_WHILE_80_C(MSGPACK_PP_BOOL(p(81, s)), p, o, s)
#define MSGPACK_PP_WHILE_81(p, o, s) MSGPACK_PP_WHILE_81_C(MSGPACK_PP_BOOL(p(82, s)), p, o, s)
#define MSGPACK_PP_WHILE_82(p, o, s) MSGPACK_PP_WHILE_82_C(MSGPACK_PP_BOOL(p(83, s)), p, o, s)
#define MSGPACK_PP_WHILE_83(p, o, s) MSGPACK_PP_WHILE_83_C(MSGPACK_PP_BOOL(p(84, s)), p, o, s)
#define MSGPACK_PP_WHILE_84(p, o, s) MSGPACK_PP_WHILE_84_C(MSGPACK_PP_BOOL(p(85, s)), p, o, s)
#define MSGPACK_PP_WHILE_85(p, o, s) MSGPACK_PP_WHILE_85_C(MSGPACK_PP_BOOL(p(86, s)), p, o, s)
#define MSGPACK_PP_WHILE_86(p, o, s) MSGPACK_PP_WHILE_86_C(MSGPACK_PP_BOOL(p(87, s)), p, o, s)
#define MSGPACK_PP_WHILE_87(p, o, s) MSGPACK_PP_WHILE_87_C(MSGPACK_PP_BOOL(p(88, s)), p, o, s)
#define MSGPACK_PP_WHILE_88(p, o, s) MSGPACK_PP_WHILE_88_C(MSGPACK_PP_BOOL(p(89, s)), p, o, s)
#define MSGPACK_PP_WHILE_89(p, o, s) MSGPACK_PP_WHILE_89_C(MSGPACK_PP_BOOL(p(90, s)), p, o, s)
#define MSGPACK_PP_WHILE_90(p, o, s) MSGPACK_PP_WHILE_90_C(MSGPACK_PP_BOOL(p(91, s)), p, o, s)
#define MSGPACK_PP_WHILE_91(p, o, s) MSGPACK_PP_WHILE_91_C(MSGPACK_PP_BOOL(p(92, s)), p, o, s)
#define MSGPACK_PP_WHILE_92(p, o, s) MSGPACK_PP_WHILE_92_C(MSGPACK_PP_BOOL(p(93, s)), p, o, s)
#define MSGPACK_PP_WHILE_93(p, o, s) MSGPACK_PP_WHILE_93_C(MSGPACK_PP_BOOL(p(94, s)), p, o, s)
#define MSGPACK_PP_WHILE_94(p, o, s) MSGPACK_PP_WHILE_94_C(MSGPACK_PP_BOOL(p(95, s)), p, o, s)
#define MSGPACK_PP_WHILE_95(p, o, s) MSGPACK_PP_WHILE_95_C(MSGPACK_PP_BOOL(p(96, s)), p, o, s)
#define MSGPACK_PP_WHILE_96(p, o, s) MSGPACK_PP_WHILE_96_C(MSGPACK_PP_BOOL(p(97, s)), p, o, s)
#define MSGPACK_PP_WHILE_97(p, o, s) MSGPACK_PP_WHILE_97_C(MSGPACK_PP_BOOL(p(98, s)), p, o, s)
#define MSGPACK_PP_WHILE_98(p, o, s) MSGPACK_PP_WHILE_98_C(MSGPACK_PP_BOOL(p(99, s)), p, o, s)
#define MSGPACK_PP_WHILE_99(p, o, s) MSGPACK_PP_WHILE_99_C(MSGPACK_PP_BOOL(p(100, s)), p, o, s)
#define MSGPACK_PP_WHILE_100(p, o, s) MSGPACK_PP_WHILE_100_C(MSGPACK_PP_BOOL(p(101, s)), p, o, s)
#define MSGPACK_PP_WHILE_101(p, o, s) MSGPACK_PP_WHILE_101_C(MSGPACK_PP_BOOL(p(102, s)), p, o, s)
#define MSGPACK_PP_WHILE_102(p, o, s) MSGPACK_PP_WHILE_102_C(MSGPACK_PP_BOOL(p(103, s)), p, o, s)
#define MSGPACK_PP_WHILE_103(p, o, s) MSGPACK_PP_WHILE_103_C(MSGPACK_PP_BOOL(p(104, s)), p, o, s)
#define MSGPACK_PP_WHILE_104(p, o, s) MSGPACK_PP_WHILE_104_C(MSGPACK_PP_BOOL(p(105, s)), p, o, s)
#define MSGPACK_PP_WHILE_105(p, o, s) MSGPACK_PP_WHILE_105_C(MSGPACK_PP_BOOL(p(106, s)), p, o, s)
#define MSGPACK_PP_WHILE_106(p, o, s) MSGPACK_PP_WHILE_106_C(MSGPACK_PP_BOOL(p(107, s)), p, o, s)
#define MSGPACK_PP_WHILE_107(p, o, s) MSGPACK_PP_WHILE_107_C(MSGPACK_PP_BOOL(p(108, s)), p, o, s)
#define MSGPACK_PP_WHILE_108(p, o, s) MSGPACK_PP_WHILE_108_C(MSGPACK_PP_BOOL(p(109, s)), p, o, s)
#define MSGPACK_PP_WHILE_109(p, o, s) MSGPACK_PP_WHILE_109_C(MSGPACK_PP_BOOL(p(110, s)), p, o, s)
#define MSGPACK_PP_WHILE_110(p, o, s) MSGPACK_PP_WHILE_110_C(MSGPACK_PP_BOOL(p(111, s)), p, o, s)
#define MSGPACK_PP_WHILE_111(p, o, s) MSGPACK_PP_WHILE_111_C(MSGPACK_PP_BOOL(p(112, s)), p, o, s)
#define MSGPACK_PP_WHILE_112(p, o, s) MSGPACK_PP_WHILE_112_C(MSGPACK_PP_BOOL(p(113, s)), p, o, s)
#define MSGPACK_PP_WHILE_113(p, o, s) MSGPACK_PP_WHILE_113_C(MSGPACK_PP_BOOL(p(114, s)), p, o, s)
#define MSGPACK_PP_WHILE_114(p, o, s) MSGPACK_PP_WHILE_114_C(MSGPACK_PP_BOOL(p(115, s)), p, o, s)
#define MSGPACK_PP_WHILE_115(p, o, s) MSGPACK_PP_WHILE_115_C(MSGPACK_PP_BOOL(p(116, s)), p, o, s)
#define MSGPACK_PP_WHILE_116(p, o, s) MSGPACK_PP_WHILE_116_C(MSGPACK_PP_BOOL(p(117, s)), p, o, s)
#define MSGPACK_PP_WHILE_117(p, o, s) MSGPACK_PP_WHILE_117_C(MSGPACK_PP_BOOL(p(118, s)), p, o, s)
#define MSGPACK_PP_WHILE_118(p, o, s) MSGPACK_PP_WHILE_118_C(MSGPACK_PP_BOOL(p(119, s)), p, o, s)
#define MSGPACK_PP_WHILE_119(p, o, s) MSGPACK_PP_WHILE_119_C(MSGPACK_PP_BOOL(p(120, s)), p, o, s)
#define MSGPACK_PP_WHILE_120(p, o, s) MSGPACK_PP_WHILE_120_C(MSGPACK_PP_BOOL(p(121, s)), p, o, s)
#define MSGPACK_PP_WHILE_121(p, o, s) MSGPACK_PP_WHILE_121_C(MSGPACK_PP_BOOL(p(122, s)), p, o, s)
#define MSGPACK_PP_WHILE_122(p, o, s) MSGPACK_PP_WHILE_122_C(MSGPACK_PP_BOOL(p(123, s)), p, o, s)
#define MSGPACK_PP_WHILE_123(p, o, s) MSGPACK_PP_WHILE_123_C(MSGPACK_PP_BOOL(p(124, s)), p, o, s)
#define MSGPACK_PP_WHILE_124(p, o, s) MSGPACK_PP_WHILE_124_C(MSGPACK_PP_BOOL(p(125, s)), p, o, s)
#define MSGPACK_PP_WHILE_125(p, o, s) MSGPACK_PP_WHILE_125_C(MSGPACK_PP_BOOL(p(126, s)), p, o, s)
#define MSGPACK_PP_WHILE_126(p, o, s) MSGPACK_PP_WHILE_126_C(MSGPACK_PP_BOOL(p(127, s)), p, o, s)
#define MSGPACK_PP_WHILE_127(p, o, s) MSGPACK_PP_WHILE_127_C(MSGPACK_PP_BOOL(p(128, s)), p, o, s)
#define MSGPACK_PP_WHILE_128(p, o, s) MSGPACK_PP_WHILE_128_C(MSGPACK_PP_BOOL(p(129, s)), p, o, s)
#define MSGPACK_PP_WHILE_129(p, o, s) MSGPACK_PP_WHILE_129_C(MSGPACK_PP_BOOL(p(130, s)), p, o, s)
#define MSGPACK_PP_WHILE_130(p, o, s) MSGPACK_PP_WHILE_130_C(MSGPACK_PP_BOOL(p(131, s)), p, o, s)
#define MSGPACK_PP_WHILE_131(p, o, s) MSGPACK_PP_WHILE_131_C(MSGPACK_PP_BOOL(p(132, s)), p, o, s)
#define MSGPACK_PP_WHILE_132(p, o, s) MSGPACK_PP_WHILE_132_C(MSGPACK_PP_BOOL(p(133, s)), p, o, s)
#define MSGPACK_PP_WHILE_133(p, o, s) MSGPACK_PP_WHILE_133_C(MSGPACK_PP_BOOL(p(134, s)), p, o, s)
#define MSGPACK_PP_WHILE_134(p, o, s) MSGPACK_PP_WHILE_134_C(MSGPACK_PP_BOOL(p(135, s)), p, o, s)
#define MSGPACK_PP_WHILE_135(p, o, s) MSGPACK_PP_WHILE_135_C(MSGPACK_PP_BOOL(p(136, s)), p, o, s)
#define MSGPACK_PP_WHILE_136(p, o, s) MSGPACK_PP_WHILE_136_C(MSGPACK_PP_BOOL(p(137, s)), p, o, s)
#define MSGPACK_PP_WHILE_137(p, o, s) MSGPACK_PP_WHILE_137_C(MSGPACK_PP_BOOL(p(138, s)), p, o, s)
#define MSGPACK_PP_WHILE_138(p, o, s) MSGPACK_PP_WHILE_138_C(MSGPACK_PP_BOOL(p(139, s)), p, o, s)
#define MSGPACK_PP_WHILE_139(p, o, s) MSGPACK_PP_WHILE_139_C(MSGPACK_PP_BOOL(p(140, s)), p, o, s)
#define MSGPACK_PP_WHILE_140(p, o, s) MSGPACK_PP_WHILE_140_C(MSGPACK_PP_BOOL(p(141, s)), p, o, s)
#define MSGPACK_PP_WHILE_141(p, o, s) MSGPACK_PP_WHILE_141_C(MSGPACK_PP_BOOL(p(142, s)), p, o, s)
#define MSGPACK_PP_WHILE_142(p, o, s) MSGPACK_PP_WHILE_142_C(MSGPACK_PP_BOOL(p(143, s)), p, o, s)
#define MSGPACK_PP_WHILE_143(p, o, s) MSGPACK_PP_WHILE_143_C(MSGPACK_PP_BOOL(p(144, s)), p, o, s)
#define MSGPACK_PP_WHILE_144(p, o, s) MSGPACK_PP_WHILE_144_C(MSGPACK_PP_BOOL(p(145, s)), p, o, s)
#define MSGPACK_PP_WHILE_145(p, o, s) MSGPACK_PP_WHILE_145_C(MSGPACK_PP_BOOL(p(146, s)), p, o, s)
#define MSGPACK_PP_WHILE_146(p, o, s) MSGPACK_PP_WHILE_146_C(MSGPACK_PP_BOOL(p(147, s)), p, o, s)
#define MSGPACK_PP_WHILE_147(p, o, s) MSGPACK_PP_WHILE_147_C(MSGPACK_PP_BOOL(p(148, s)), p, o, s)
#define MSGPACK_PP_WHILE_148(p, o, s) MSGPACK_PP_WHILE_148_C(MSGPACK_PP_BOOL(p(149, s)), p, o, s)
#define MSGPACK_PP_WHILE_149(p, o, s) MSGPACK_PP_WHILE_149_C(MSGPACK_PP_BOOL(p(150, s)), p, o, s)
#define MSGPACK_PP_WHILE_150(p, o, s) MSGPACK_PP_WHILE_150_C(MSGPACK_PP_BOOL(p(151, s)), p, o, s)
#define MSGPACK_PP_WHILE_151(p, o, s) MSGPACK_PP_WHILE_151_C(MSGPACK_PP_BOOL(p(152, s)), p, o, s)
#define MSGPACK_PP_WHILE_152(p, o, s) MSGPACK_PP_WHILE_152_C(MSGPACK_PP_BOOL(p(153, s)), p, o, s)
#define MSGPACK_PP_WHILE_153(p, o, s) MSGPACK_PP_WHILE_153_C(MSGPACK_PP_BOOL(p(154, s)), p, o, s)
#define MSGPACK_PP_WHILE_154(p, o, s) MSGPACK_PP_WHILE_154_C(MSGPACK_PP_BOOL(p(155, s)), p, o, s)
#define MSGPACK_PP_WHILE_155(p, o, s) MSGPACK_PP_WHILE_155_C(MSGPACK_PP_BOOL(p(156, s)), p, o, s)
#define MSGPACK_PP_WHILE_156(p, o, s) MSGPACK_PP_WHILE_156_C(MSGPACK_PP_BOOL(p(157, s)), p, o, s)
#define MSGPACK_PP_WHILE_157(p, o, s) MSGPACK_PP_WHILE_157_C(MSGPACK_PP_BOOL(p(158, s)), p, o, s)
#define MSGPACK_PP_WHILE_158(p, o, s) MSGPACK_PP_WHILE_158_C(MSGPACK_PP_BOOL(p(159, s)), p, o, s)
#define MSGPACK_PP_WHILE_159(p, o, s) MSGPACK_PP_WHILE_159_C(MSGPACK_PP_BOOL(p(160, s)), p, o, s)
#define MSGPACK_PP_WHILE_160(p, o, s) MSGPACK_PP_WHILE_160_C(MSGPACK_PP_BOOL(p(161, s)), p, o, s)
#define MSGPACK_PP_WHILE_161(p, o, s) MSGPACK_PP_WHILE_161_C(MSGPACK_PP_BOOL(p(162, s)), p, o, s)
#define MSGPACK_PP_WHILE_162(p, o, s) MSGPACK_PP_WHILE_162_C(MSGPACK_PP_BOOL(p(163, s)), p, o, s)
#define MSGPACK_PP_WHILE_163(p, o, s) MSGPACK_PP_WHILE_163_C(MSGPACK_PP_BOOL(p(164, s)), p, o, s)
#define MSGPACK_PP_WHILE_164(p, o, s) MSGPACK_PP_WHILE_164_C(MSGPACK_PP_BOOL(p(165, s)), p, o, s)
#define MSGPACK_PP_WHILE_165(p, o, s) MSGPACK_PP_WHILE_165_C(MSGPACK_PP_BOOL(p(166, s)), p, o, s)
#define MSGPACK_PP_WHILE_166(p, o, s) MSGPACK_PP_WHILE_166_C(MSGPACK_PP_BOOL(p(167, s)), p, o, s)
#define MSGPACK_PP_WHILE_167(p, o, s) MSGPACK_PP_WHILE_167_C(MSGPACK_PP_BOOL(p(168, s)), p, o, s)
#define MSGPACK_PP_WHILE_168(p, o, s) MSGPACK_PP_WHILE_168_C(MSGPACK_PP_BOOL(p(169, s)), p, o, s)
#define MSGPACK_PP_WHILE_169(p, o, s) MSGPACK_PP_WHILE_169_C(MSGPACK_PP_BOOL(p(170, s)), p, o, s)
#define MSGPACK_PP_WHILE_170(p, o, s) MSGPACK_PP_WHILE_170_C(MSGPACK_PP_BOOL(p(171, s)), p, o, s)
#define MSGPACK_PP_WHILE_171(p, o, s) MSGPACK_PP_WHILE_171_C(MSGPACK_PP_BOOL(p(172, s)), p, o, s)
#define MSGPACK_PP_WHILE_172(p, o, s) MSGPACK_PP_WHILE_172_C(MSGPACK_PP_BOOL(p(173, s)), p, o, s)
#define MSGPACK_PP_WHILE_173(p, o, s) MSGPACK_PP_WHILE_173_C(MSGPACK_PP_BOOL(p(174, s)), p, o, s)
#define MSGPACK_PP_WHILE_174(p, o, s) MSGPACK_PP_WHILE_174_C(MSGPACK_PP_BOOL(p(175, s)), p, o, s)
#define MSGPACK_PP_WHILE_175(p, o, s) MSGPACK_PP_WHILE_175_C(MSGPACK_PP_BOOL(p(176, s)), p, o, s)
#define MSGPACK_PP_WHILE_176(p, o, s) MSGPACK_PP_WHILE_176_C(MSGPACK_PP_BOOL(p(177, s)), p, o, s)
#define MSGPACK_PP_WHILE_177(p, o, s) MSGPACK_PP_WHILE_177_C(MSGPACK_PP_BOOL(p(178, s)), p, o, s)
#define MSGPACK_PP_WHILE_178(p, o, s) MSGPACK_PP_WHILE_178_C(MSGPACK_PP_BOOL(p(179, s)), p, o, s)
#define MSGPACK_PP_WHILE_179(p, o, s) MSGPACK_PP_WHILE_179_C(MSGPACK_PP_BOOL(p(180, s)), p, o, s)
#define MSGPACK_PP_WHILE_180(p, o, s) MSGPACK_PP_WHILE_180_C(MSGPACK_PP_BOOL(p(181, s)), p, o, s)
#define MSGPACK_PP_WHILE_181(p, o, s) MSGPACK_PP_WHILE_181_C(MSGPACK_PP_BOOL(p(182, s)), p, o, s)
#define MSGPACK_PP_WHILE_182(p, o, s) MSGPACK_PP_WHILE_182_C(MSGPACK_PP_BOOL(p(183, s)), p, o, s)
#define MSGPACK_PP_WHILE_183(p, o, s) MSGPACK_PP_WHILE_183_C(MSGPACK_PP_BOOL(p(184, s)), p, o, s)
#define MSGPACK_PP_WHILE_184(p, o, s) MSGPACK_PP_WHILE_184_C(MSGPACK_PP_BOOL(p(185, s)), p, o, s)
#define MSGPACK_PP_WHILE_185(p, o, s) MSGPACK_PP_WHILE_185_C(MSGPACK_PP_BOOL(p(186, s)), p, o, s)
#define MSGPACK_PP_WHILE_186(p, o, s) MSGPACK_PP_WHILE_186_C(MSGPACK_PP_BOOL(p(187, s)), p, o, s)
#define MSGPACK_PP_WHILE_187(p, o, s) MSGPACK_PP_WHILE_187_C(MSGPACK_PP_BOOL(p(188, s)), p, o, s)
#define MSGPACK_PP_WHILE_188(p, o, s) MSGPACK_PP_WHILE_188_C(MSGPACK_PP_BOOL(p(189, s)), p, o, s)
#define MSGPACK_PP_WHILE_189(p, o, s) MSGPACK_PP_WHILE_189_C(MSGPACK_PP_BOOL(p(190, s)), p, o, s)
#define MSGPACK_PP_WHILE_190(p, o, s) MSGPACK_PP_WHILE_190_C(MSGPACK_PP_BOOL(p(191, s)), p, o, s)
#define MSGPACK_PP_WHILE_191(p, o, s) MSGPACK_PP_WHILE_191_C(MSGPACK_PP_BOOL(p(192, s)), p, o, s)
#define MSGPACK_PP_WHILE_192(p, o, s) MSGPACK_PP_WHILE_192_C(MSGPACK_PP_BOOL(p(193, s)), p, o, s)
#define MSGPACK_PP_WHILE_193(p, o, s) MSGPACK_PP_WHILE_193_C(MSGPACK_PP_BOOL(p(194, s)), p, o, s)
#define MSGPACK_PP_WHILE_194(p, o, s) MSGPACK_PP_WHILE_194_C(MSGPACK_PP_BOOL(p(195, s)), p, o, s)
#define MSGPACK_PP_WHILE_195(p, o, s) MSGPACK_PP_WHILE_195_C(MSGPACK_PP_BOOL(p(196, s)), p, o, s)
#define MSGPACK_PP_WHILE_196(p, o, s) MSGPACK_PP_WHILE_196_C(MSGPACK_PP_BOOL(p(197, s)), p, o, s)
#define MSGPACK_PP_WHILE_197(p, o, s) MSGPACK_PP_WHILE_197_C(MSGPACK_PP_BOOL(p(198, s)), p, o, s)
#define MSGPACK_PP_WHILE_198(p, o, s) MSGPACK_PP_WHILE_198_C(MSGPACK_PP_BOOL(p(199, s)), p, o, s)
#define MSGPACK_PP_WHILE_199(p, o, s) MSGPACK_PP_WHILE_199_C(MSGPACK_PP_BOOL(p(200, s)), p, o, s)
#define MSGPACK_PP_WHILE_200(p, o, s) MSGPACK_PP_WHILE_200_C(MSGPACK_PP_BOOL(p(201, s)), p, o, s)
#define MSGPACK_PP_WHILE_201(p, o, s) MSGPACK_PP_WHILE_201_C(MSGPACK_PP_BOOL(p(202, s)), p, o, s)
#define MSGPACK_PP_WHILE_202(p, o, s) MSGPACK_PP_WHILE_202_C(MSGPACK_PP_BOOL(p(203, s)), p, o, s)
#define MSGPACK_PP_WHILE_203(p, o, s) MSGPACK_PP_WHILE_203_C(MSGPACK_PP_BOOL(p(204, s)), p, o, s)
#define MSGPACK_PP_WHILE_204(p, o, s) MSGPACK_PP_WHILE_204_C(MSGPACK_PP_BOOL(p(205, s)), p, o, s)
#define MSGPACK_PP_WHILE_205(p, o, s) MSGPACK_PP_WHILE_205_C(MSGPACK_PP_BOOL(p(206, s)), p, o, s)
#define MSGPACK_PP_WHILE_206(p, o, s) MSGPACK_PP_WHILE_206_C(MSGPACK_PP_BOOL(p(207, s)), p, o, s)
#define MSGPACK_PP_WHILE_207(p, o, s) MSGPACK_PP_WHILE_207_C(MSGPACK_PP_BOOL(p(208, s)), p, o, s)
#define MSGPACK_PP_WHILE_208(p, o, s) MSGPACK_PP_WHILE_208_C(MSGPACK_PP_BOOL(p(209, s)), p, o, s)
#define MSGPACK_PP_WHILE_209(p, o, s) MSGPACK_PP_WHILE_209_C(MSGPACK_PP_BOOL(p(210, s)), p, o, s)
#define MSGPACK_PP_WHILE_210(p, o, s) MSGPACK_PP_WHILE_210_C(MSGPACK_PP_BOOL(p(211, s)), p, o, s)
#define MSGPACK_PP_WHILE_211(p, o, s) MSGPACK_PP_WHILE_211_C(MSGPACK_PP_BOOL(p(212, s)), p, o, s)
#define MSGPACK_PP_WHILE_212(p, o, s) MSGPACK_PP_WHILE_212_C(MSGPACK_PP_BOOL(p(213, s)), p, o, s)
#define MSGPACK_PP_WHILE_213(p, o, s) MSGPACK_PP_WHILE_213_C(MSGPACK_PP_BOOL(p(214, s)), p, o, s)
#define MSGPACK_PP_WHILE_214(p, o, s) MSGPACK_PP_WHILE_214_C(MSGPACK_PP_BOOL(p(215, s)), p, o, s)
#define MSGPACK_PP_WHILE_215(p, o, s) MSGPACK_PP_WHILE_215_C(MSGPACK_PP_BOOL(p(216, s)), p, o, s)
#define MSGPACK_PP_WHILE_216(p, o, s) MSGPACK_PP_WHILE_216_C(MSGPACK_PP_BOOL(p(217, s)), p, o, s)
#define MSGPACK_PP_WHILE_217(p, o, s) MSGPACK_PP_WHILE_217_C(MSGPACK_PP_BOOL(p(218, s)), p, o, s)
#define MSGPACK_PP_WHILE_218(p, o, s) MSGPACK_PP_WHILE_218_C(MSGPACK_PP_BOOL(p(219, s)), p, o, s)
#define MSGPACK_PP_WHILE_219(p, o, s) MSGPACK_PP_WHILE_219_C(MSGPACK_PP_BOOL(p(220, s)), p, o, s)
#define MSGPACK_PP_WHILE_220(p, o, s) MSGPACK_PP_WHILE_220_C(MSGPACK_PP_BOOL(p(221, s)), p, o, s)
#define MSGPACK_PP_WHILE_221(p, o, s) MSGPACK_PP_WHILE_221_C(MSGPACK_PP_BOOL(p(222, s)), p, o, s)
#define MSGPACK_PP_WHILE_222(p, o, s) MSGPACK_PP_WHILE_222_C(MSGPACK_PP_BOOL(p(223, s)), p, o, s)
#define MSGPACK_PP_WHILE_223(p, o, s) MSGPACK_PP_WHILE_223_C(MSGPACK_PP_BOOL(p(224, s)), p, o, s)
#define MSGPACK_PP_WHILE_224(p, o, s) MSGPACK_PP_WHILE_224_C(MSGPACK_PP_BOOL(p(225, s)), p, o, s)
#define MSGPACK_PP_WHILE_225(p, o, s) MSGPACK_PP_WHILE_225_C(MSGPACK_PP_BOOL(p(226, s)), p, o, s)
#define MSGPACK_PP_WHILE_226(p, o, s) MSGPACK_PP_WHILE_226_C(MSGPACK_PP_BOOL(p(227, s)), p, o, s)
#define MSGPACK_PP_WHILE_227(p, o, s) MSGPACK_PP_WHILE_227_C(MSGPACK_PP_BOOL(p(228, s)), p, o, s)
#define MSGPACK_PP_WHILE_228(p, o, s) MSGPACK_PP_WHILE_228_C(MSGPACK_PP_BOOL(p(229, s)), p, o, s)
#define MSGPACK_PP_WHILE_229(p, o, s) MSGPACK_PP_WHILE_229_C(MSGPACK_PP_BOOL(p(230, s)), p, o, s)
#define MSGPACK_PP_WHILE_230(p, o, s) MSGPACK_PP_WHILE_230_C(MSGPACK_PP_BOOL(p(231, s)), p, o, s)
#define MSGPACK_PP_WHILE_231(p, o, s) MSGPACK_PP_WHILE_231_C(MSGPACK_PP_BOOL(p(232, s)), p, o, s)
#define MSGPACK_PP_WHILE_232(p, o, s) MSGPACK_PP_WHILE_232_C(MSGPACK_PP_BOOL(p(233, s)), p, o, s)
#define MSGPACK_PP_WHILE_233(p, o, s) MSGPACK_PP_WHILE_233_C(MSGPACK_PP_BOOL(p(234, s)), p, o, s)
#define MSGPACK_PP_WHILE_234(p, o, s) MSGPACK_PP_WHILE_234_C(MSGPACK_PP_BOOL(p(235, s)), p, o, s)
#define MSGPACK_PP_WHILE_235(p, o, s) MSGPACK_PP_WHILE_235_C(MSGPACK_PP_BOOL(p(236, s)), p, o, s)
#define MSGPACK_PP_WHILE_236(p, o, s) MSGPACK_PP_WHILE_236_C(MSGPACK_PP_BOOL(p(237, s)), p, o, s)
#define MSGPACK_PP_WHILE_237(p, o, s) MSGPACK_PP_WHILE_237_C(MSGPACK_PP_BOOL(p(238, s)), p, o, s)
#define MSGPACK_PP_WHILE_238(p, o, s) MSGPACK_PP_WHILE_238_C(MSGPACK_PP_BOOL(p(239, s)), p, o, s)
#define MSGPACK_PP_WHILE_239(p, o, s) MSGPACK_PP_WHILE_239_C(MSGPACK_PP_BOOL(p(240, s)), p, o, s)
#define MSGPACK_PP_WHILE_240(p, o, s) MSGPACK_PP_WHILE_240_C(MSGPACK_PP_BOOL(p(241, s)), p, o, s)
#define MSGPACK_PP_WHILE_241(p, o, s) MSGPACK_PP_WHILE_241_C(MSGPACK_PP_BOOL(p(242, s)), p, o, s)
#define MSGPACK_PP_WHILE_242(p, o, s) MSGPACK_PP_WHILE_242_C(MSGPACK_PP_BOOL(p(243, s)), p, o, s)
#define MSGPACK_PP_WHILE_243(p, o, s) MSGPACK_PP_WHILE_243_C(MSGPACK_PP_BOOL(p(244, s)), p, o, s)
#define MSGPACK_PP_WHILE_244(p, o, s) MSGPACK_PP_WHILE_244_C(MSGPACK_PP_BOOL(p(245, s)), p, o, s)
#define MSGPACK_PP_WHILE_245(p, o, s) MSGPACK_PP_WHILE_245_C(MSGPACK_PP_BOOL(p(246, s)), p, o, s)
#define MSGPACK_PP_WHILE_246(p, o, s) MSGPACK_PP_WHILE_246_C(MSGPACK_PP_BOOL(p(247, s)), p, o, s)
#define MSGPACK_PP_WHILE_247(p, o, s) MSGPACK_PP_WHILE_247_C(MSGPACK_PP_BOOL(p(248, s)), p, o, s)
#define MSGPACK_PP_WHILE_248(p, o, s) MSGPACK_PP_WHILE_248_C(MSGPACK_PP_BOOL(p(249, s)), p, o, s)
#define MSGPACK_PP_WHILE_249(p, o, s) MSGPACK_PP_WHILE_249_C(MSGPACK_PP_BOOL(p(250, s)), p, o, s)
#define MSGPACK_PP_WHILE_250(p, o, s) MSGPACK_PP_WHILE_250_C(MSGPACK_PP_BOOL(p(251, s)), p, o, s)
#define MSGPACK_PP_WHILE_251(p, o, s) MSGPACK_PP_WHILE_251_C(MSGPACK_PP_BOOL(p(252, s)), p, o, s)
#define MSGPACK_PP_WHILE_252(p, o, s) MSGPACK_PP_WHILE_252_C(MSGPACK_PP_BOOL(p(253, s)), p, o, s)
#define MSGPACK_PP_WHILE_253(p, o, s) MSGPACK_PP_WHILE_253_C(MSGPACK_PP_BOOL(p(254, s)), p, o, s)
#define MSGPACK_PP_WHILE_254(p, o, s) MSGPACK_PP_WHILE_254_C(MSGPACK_PP_BOOL(p(255, s)), p, o, s)
#define MSGPACK_PP_WHILE_255(p, o, s) MSGPACK_PP_WHILE_255_C(MSGPACK_PP_BOOL(p(256, s)), p, o, s)
#define MSGPACK_PP_WHILE_256(p, o, s) MSGPACK_PP_WHILE_256_C(MSGPACK_PP_BOOL(p(257, s)), p, o, s)
#
#define MSGPACK_PP_WHILE_1_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_2, s MSGPACK_PP_TUPLE_EAT_3)                                                    \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(2, s))
#define MSGPACK_PP_WHILE_2_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_3, s MSGPACK_PP_TUPLE_EAT_3)                                                    \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(3, s))
#define MSGPACK_PP_WHILE_3_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_4, s MSGPACK_PP_TUPLE_EAT_3)                                                    \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(4, s))
#define MSGPACK_PP_WHILE_4_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_5, s MSGPACK_PP_TUPLE_EAT_3)                                                    \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(5, s))
#define MSGPACK_PP_WHILE_5_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_6, s MSGPACK_PP_TUPLE_EAT_3)                                                    \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(6, s))
#define MSGPACK_PP_WHILE_6_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_7, s MSGPACK_PP_TUPLE_EAT_3)                                                    \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(7, s))
#define MSGPACK_PP_WHILE_7_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_8, s MSGPACK_PP_TUPLE_EAT_3)                                                    \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(8, s))
#define MSGPACK_PP_WHILE_8_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_9, s MSGPACK_PP_TUPLE_EAT_3)                                                    \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(9, s))
#define MSGPACK_PP_WHILE_9_C(c, p, o, s)                                                                               \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_10, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(10, s))
#define MSGPACK_PP_WHILE_10_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_11, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(11, s))
#define MSGPACK_PP_WHILE_11_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_12, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(12, s))
#define MSGPACK_PP_WHILE_12_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_13, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(13, s))
#define MSGPACK_PP_WHILE_13_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_14, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(14, s))
#define MSGPACK_PP_WHILE_14_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_15, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(15, s))
#define MSGPACK_PP_WHILE_15_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_16, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(16, s))
#define MSGPACK_PP_WHILE_16_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_17, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(17, s))
#define MSGPACK_PP_WHILE_17_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_18, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(18, s))
#define MSGPACK_PP_WHILE_18_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_19, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(19, s))
#define MSGPACK_PP_WHILE_19_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_20, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(20, s))
#define MSGPACK_PP_WHILE_20_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_21, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(21, s))
#define MSGPACK_PP_WHILE_21_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_22, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(22, s))
#define MSGPACK_PP_WHILE_22_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_23, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(23, s))
#define MSGPACK_PP_WHILE_23_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_24, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(24, s))
#define MSGPACK_PP_WHILE_24_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_25, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(25, s))
#define MSGPACK_PP_WHILE_25_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_26, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(26, s))
#define MSGPACK_PP_WHILE_26_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_27, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(27, s))
#define MSGPACK_PP_WHILE_27_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_28, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(28, s))
#define MSGPACK_PP_WHILE_28_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_29, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(29, s))
#define MSGPACK_PP_WHILE_29_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_30, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(30, s))
#define MSGPACK_PP_WHILE_30_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_31, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(31, s))
#define MSGPACK_PP_WHILE_31_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_32, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(32, s))
#define MSGPACK_PP_WHILE_32_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_33, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(33, s))
#define MSGPACK_PP_WHILE_33_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_34, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(34, s))
#define MSGPACK_PP_WHILE_34_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_35, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(35, s))
#define MSGPACK_PP_WHILE_35_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_36, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(36, s))
#define MSGPACK_PP_WHILE_36_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_37, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(37, s))
#define MSGPACK_PP_WHILE_37_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_38, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(38, s))
#define MSGPACK_PP_WHILE_38_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_39, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(39, s))
#define MSGPACK_PP_WHILE_39_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_40, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(40, s))
#define MSGPACK_PP_WHILE_40_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_41, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(41, s))
#define MSGPACK_PP_WHILE_41_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_42, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(42, s))
#define MSGPACK_PP_WHILE_42_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_43, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(43, s))
#define MSGPACK_PP_WHILE_43_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_44, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(44, s))
#define MSGPACK_PP_WHILE_44_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_45, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(45, s))
#define MSGPACK_PP_WHILE_45_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_46, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(46, s))
#define MSGPACK_PP_WHILE_46_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_47, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(47, s))
#define MSGPACK_PP_WHILE_47_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_48, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(48, s))
#define MSGPACK_PP_WHILE_48_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_49, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(49, s))
#define MSGPACK_PP_WHILE_49_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_50, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(50, s))
#define MSGPACK_PP_WHILE_50_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_51, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(51, s))
#define MSGPACK_PP_WHILE_51_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_52, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(52, s))
#define MSGPACK_PP_WHILE_52_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_53, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(53, s))
#define MSGPACK_PP_WHILE_53_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_54, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(54, s))
#define MSGPACK_PP_WHILE_54_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_55, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(55, s))
#define MSGPACK_PP_WHILE_55_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_56, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(56, s))
#define MSGPACK_PP_WHILE_56_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_57, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(57, s))
#define MSGPACK_PP_WHILE_57_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_58, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(58, s))
#define MSGPACK_PP_WHILE_58_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_59, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(59, s))
#define MSGPACK_PP_WHILE_59_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_60, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(60, s))
#define MSGPACK_PP_WHILE_60_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_61, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(61, s))
#define MSGPACK_PP_WHILE_61_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_62, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(62, s))
#define MSGPACK_PP_WHILE_62_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_63, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(63, s))
#define MSGPACK_PP_WHILE_63_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_64, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(64, s))
#define MSGPACK_PP_WHILE_64_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_65, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(65, s))
#define MSGPACK_PP_WHILE_65_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_66, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(66, s))
#define MSGPACK_PP_WHILE_66_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_67, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(67, s))
#define MSGPACK_PP_WHILE_67_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_68, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(68, s))
#define MSGPACK_PP_WHILE_68_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_69, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(69, s))
#define MSGPACK_PP_WHILE_69_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_70, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(70, s))
#define MSGPACK_PP_WHILE_70_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_71, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(71, s))
#define MSGPACK_PP_WHILE_71_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_72, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(72, s))
#define MSGPACK_PP_WHILE_72_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_73, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(73, s))
#define MSGPACK_PP_WHILE_73_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_74, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(74, s))
#define MSGPACK_PP_WHILE_74_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_75, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(75, s))
#define MSGPACK_PP_WHILE_75_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_76, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(76, s))
#define MSGPACK_PP_WHILE_76_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_77, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(77, s))
#define MSGPACK_PP_WHILE_77_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_78, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(78, s))
#define MSGPACK_PP_WHILE_78_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_79, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(79, s))
#define MSGPACK_PP_WHILE_79_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_80, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(80, s))
#define MSGPACK_PP_WHILE_80_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_81, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(81, s))
#define MSGPACK_PP_WHILE_81_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_82, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(82, s))
#define MSGPACK_PP_WHILE_82_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_83, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(83, s))
#define MSGPACK_PP_WHILE_83_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_84, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(84, s))
#define MSGPACK_PP_WHILE_84_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_85, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(85, s))
#define MSGPACK_PP_WHILE_85_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_86, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(86, s))
#define MSGPACK_PP_WHILE_86_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_87, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(87, s))
#define MSGPACK_PP_WHILE_87_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_88, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(88, s))
#define MSGPACK_PP_WHILE_88_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_89, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(89, s))
#define MSGPACK_PP_WHILE_89_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_90, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(90, s))
#define MSGPACK_PP_WHILE_90_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_91, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(91, s))
#define MSGPACK_PP_WHILE_91_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_92, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(92, s))
#define MSGPACK_PP_WHILE_92_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_93, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(93, s))
#define MSGPACK_PP_WHILE_93_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_94, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(94, s))
#define MSGPACK_PP_WHILE_94_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_95, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(95, s))
#define MSGPACK_PP_WHILE_95_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_96, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(96, s))
#define MSGPACK_PP_WHILE_96_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_97, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(97, s))
#define MSGPACK_PP_WHILE_97_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_98, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(98, s))
#define MSGPACK_PP_WHILE_98_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_99, s MSGPACK_PP_TUPLE_EAT_3)                                                   \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(99, s))
#define MSGPACK_PP_WHILE_99_C(c, p, o, s)                                                                              \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_100, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(100, s))
#define MSGPACK_PP_WHILE_100_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_101, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(101, s))
#define MSGPACK_PP_WHILE_101_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_102, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(102, s))
#define MSGPACK_PP_WHILE_102_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_103, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(103, s))
#define MSGPACK_PP_WHILE_103_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_104, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(104, s))
#define MSGPACK_PP_WHILE_104_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_105, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(105, s))
#define MSGPACK_PP_WHILE_105_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_106, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(106, s))
#define MSGPACK_PP_WHILE_106_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_107, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(107, s))
#define MSGPACK_PP_WHILE_107_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_108, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(108, s))
#define MSGPACK_PP_WHILE_108_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_109, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(109, s))
#define MSGPACK_PP_WHILE_109_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_110, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(110, s))
#define MSGPACK_PP_WHILE_110_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_111, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(111, s))
#define MSGPACK_PP_WHILE_111_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_112, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(112, s))
#define MSGPACK_PP_WHILE_112_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_113, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(113, s))
#define MSGPACK_PP_WHILE_113_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_114, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(114, s))
#define MSGPACK_PP_WHILE_114_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_115, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(115, s))
#define MSGPACK_PP_WHILE_115_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_116, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(116, s))
#define MSGPACK_PP_WHILE_116_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_117, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(117, s))
#define MSGPACK_PP_WHILE_117_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_118, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(118, s))
#define MSGPACK_PP_WHILE_118_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_119, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(119, s))
#define MSGPACK_PP_WHILE_119_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_120, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(120, s))
#define MSGPACK_PP_WHILE_120_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_121, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(121, s))
#define MSGPACK_PP_WHILE_121_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_122, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(122, s))
#define MSGPACK_PP_WHILE_122_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_123, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(123, s))
#define MSGPACK_PP_WHILE_123_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_124, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(124, s))
#define MSGPACK_PP_WHILE_124_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_125, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(125, s))
#define MSGPACK_PP_WHILE_125_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_126, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(126, s))
#define MSGPACK_PP_WHILE_126_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_127, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(127, s))
#define MSGPACK_PP_WHILE_127_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_128, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(128, s))
#define MSGPACK_PP_WHILE_128_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_129, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(129, s))
#define MSGPACK_PP_WHILE_129_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_130, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(130, s))
#define MSGPACK_PP_WHILE_130_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_131, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(131, s))
#define MSGPACK_PP_WHILE_131_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_132, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(132, s))
#define MSGPACK_PP_WHILE_132_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_133, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(133, s))
#define MSGPACK_PP_WHILE_133_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_134, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(134, s))
#define MSGPACK_PP_WHILE_134_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_135, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(135, s))
#define MSGPACK_PP_WHILE_135_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_136, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(136, s))
#define MSGPACK_PP_WHILE_136_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_137, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(137, s))
#define MSGPACK_PP_WHILE_137_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_138, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(138, s))
#define MSGPACK_PP_WHILE_138_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_139, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(139, s))
#define MSGPACK_PP_WHILE_139_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_140, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(140, s))
#define MSGPACK_PP_WHILE_140_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_141, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(141, s))
#define MSGPACK_PP_WHILE_141_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_142, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(142, s))
#define MSGPACK_PP_WHILE_142_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_143, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(143, s))
#define MSGPACK_PP_WHILE_143_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_144, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(144, s))
#define MSGPACK_PP_WHILE_144_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_145, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(145, s))
#define MSGPACK_PP_WHILE_145_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_146, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(146, s))
#define MSGPACK_PP_WHILE_146_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_147, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(147, s))
#define MSGPACK_PP_WHILE_147_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_148, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(148, s))
#define MSGPACK_PP_WHILE_148_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_149, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(149, s))
#define MSGPACK_PP_WHILE_149_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_150, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(150, s))
#define MSGPACK_PP_WHILE_150_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_151, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(151, s))
#define MSGPACK_PP_WHILE_151_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_152, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(152, s))
#define MSGPACK_PP_WHILE_152_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_153, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(153, s))
#define MSGPACK_PP_WHILE_153_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_154, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(154, s))
#define MSGPACK_PP_WHILE_154_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_155, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(155, s))
#define MSGPACK_PP_WHILE_155_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_156, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(156, s))
#define MSGPACK_PP_WHILE_156_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_157, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(157, s))
#define MSGPACK_PP_WHILE_157_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_158, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(158, s))
#define MSGPACK_PP_WHILE_158_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_159, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(159, s))
#define MSGPACK_PP_WHILE_159_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_160, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(160, s))
#define MSGPACK_PP_WHILE_160_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_161, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(161, s))
#define MSGPACK_PP_WHILE_161_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_162, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(162, s))
#define MSGPACK_PP_WHILE_162_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_163, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(163, s))
#define MSGPACK_PP_WHILE_163_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_164, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(164, s))
#define MSGPACK_PP_WHILE_164_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_165, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(165, s))
#define MSGPACK_PP_WHILE_165_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_166, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(166, s))
#define MSGPACK_PP_WHILE_166_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_167, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(167, s))
#define MSGPACK_PP_WHILE_167_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_168, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(168, s))
#define MSGPACK_PP_WHILE_168_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_169, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(169, s))
#define MSGPACK_PP_WHILE_169_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_170, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(170, s))
#define MSGPACK_PP_WHILE_170_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_171, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(171, s))
#define MSGPACK_PP_WHILE_171_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_172, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(172, s))
#define MSGPACK_PP_WHILE_172_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_173, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(173, s))
#define MSGPACK_PP_WHILE_173_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_174, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(174, s))
#define MSGPACK_PP_WHILE_174_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_175, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(175, s))
#define MSGPACK_PP_WHILE_175_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_176, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(176, s))
#define MSGPACK_PP_WHILE_176_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_177, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(177, s))
#define MSGPACK_PP_WHILE_177_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_178, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(178, s))
#define MSGPACK_PP_WHILE_178_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_179, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(179, s))
#define MSGPACK_PP_WHILE_179_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_180, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(180, s))
#define MSGPACK_PP_WHILE_180_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_181, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(181, s))
#define MSGPACK_PP_WHILE_181_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_182, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(182, s))
#define MSGPACK_PP_WHILE_182_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_183, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(183, s))
#define MSGPACK_PP_WHILE_183_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_184, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(184, s))
#define MSGPACK_PP_WHILE_184_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_185, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(185, s))
#define MSGPACK_PP_WHILE_185_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_186, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(186, s))
#define MSGPACK_PP_WHILE_186_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_187, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(187, s))
#define MSGPACK_PP_WHILE_187_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_188, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(188, s))
#define MSGPACK_PP_WHILE_188_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_189, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(189, s))
#define MSGPACK_PP_WHILE_189_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_190, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(190, s))
#define MSGPACK_PP_WHILE_190_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_191, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(191, s))
#define MSGPACK_PP_WHILE_191_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_192, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(192, s))
#define MSGPACK_PP_WHILE_192_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_193, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(193, s))
#define MSGPACK_PP_WHILE_193_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_194, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(194, s))
#define MSGPACK_PP_WHILE_194_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_195, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(195, s))
#define MSGPACK_PP_WHILE_195_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_196, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(196, s))
#define MSGPACK_PP_WHILE_196_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_197, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(197, s))
#define MSGPACK_PP_WHILE_197_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_198, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(198, s))
#define MSGPACK_PP_WHILE_198_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_199, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(199, s))
#define MSGPACK_PP_WHILE_199_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_200, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(200, s))
#define MSGPACK_PP_WHILE_200_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_201, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(201, s))
#define MSGPACK_PP_WHILE_201_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_202, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(202, s))
#define MSGPACK_PP_WHILE_202_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_203, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(203, s))
#define MSGPACK_PP_WHILE_203_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_204, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(204, s))
#define MSGPACK_PP_WHILE_204_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_205, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(205, s))
#define MSGPACK_PP_WHILE_205_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_206, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(206, s))
#define MSGPACK_PP_WHILE_206_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_207, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(207, s))
#define MSGPACK_PP_WHILE_207_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_208, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(208, s))
#define MSGPACK_PP_WHILE_208_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_209, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(209, s))
#define MSGPACK_PP_WHILE_209_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_210, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(210, s))
#define MSGPACK_PP_WHILE_210_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_211, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(211, s))
#define MSGPACK_PP_WHILE_211_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_212, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(212, s))
#define MSGPACK_PP_WHILE_212_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_213, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(213, s))
#define MSGPACK_PP_WHILE_213_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_214, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(214, s))
#define MSGPACK_PP_WHILE_214_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_215, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(215, s))
#define MSGPACK_PP_WHILE_215_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_216, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(216, s))
#define MSGPACK_PP_WHILE_216_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_217, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(217, s))
#define MSGPACK_PP_WHILE_217_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_218, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(218, s))
#define MSGPACK_PP_WHILE_218_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_219, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(219, s))
#define MSGPACK_PP_WHILE_219_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_220, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(220, s))
#define MSGPACK_PP_WHILE_220_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_221, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(221, s))
#define MSGPACK_PP_WHILE_221_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_222, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(222, s))
#define MSGPACK_PP_WHILE_222_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_223, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(223, s))
#define MSGPACK_PP_WHILE_223_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_224, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(224, s))
#define MSGPACK_PP_WHILE_224_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_225, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(225, s))
#define MSGPACK_PP_WHILE_225_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_226, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(226, s))
#define MSGPACK_PP_WHILE_226_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_227, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(227, s))
#define MSGPACK_PP_WHILE_227_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_228, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(228, s))
#define MSGPACK_PP_WHILE_228_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_229, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(229, s))
#define MSGPACK_PP_WHILE_229_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_230, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(230, s))
#define MSGPACK_PP_WHILE_230_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_231, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(231, s))
#define MSGPACK_PP_WHILE_231_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_232, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(232, s))
#define MSGPACK_PP_WHILE_232_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_233, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(233, s))
#define MSGPACK_PP_WHILE_233_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_234, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(234, s))
#define MSGPACK_PP_WHILE_234_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_235, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(235, s))
#define MSGPACK_PP_WHILE_235_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_236, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(236, s))
#define MSGPACK_PP_WHILE_236_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_237, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(237, s))
#define MSGPACK_PP_WHILE_237_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_238, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(238, s))
#define MSGPACK_PP_WHILE_238_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_239, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(239, s))
#define MSGPACK_PP_WHILE_239_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_240, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(240, s))
#define MSGPACK_PP_WHILE_240_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_241, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(241, s))
#define MSGPACK_PP_WHILE_241_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_242, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(242, s))
#define MSGPACK_PP_WHILE_242_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_243, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(243, s))
#define MSGPACK_PP_WHILE_243_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_244, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(244, s))
#define MSGPACK_PP_WHILE_244_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_245, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(245, s))
#define MSGPACK_PP_WHILE_245_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_246, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(246, s))
#define MSGPACK_PP_WHILE_246_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_247, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(247, s))
#define MSGPACK_PP_WHILE_247_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_248, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(248, s))
#define MSGPACK_PP_WHILE_248_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_249, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(249, s))
#define MSGPACK_PP_WHILE_249_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_250, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(250, s))
#define MSGPACK_PP_WHILE_250_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_251, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(251, s))
#define MSGPACK_PP_WHILE_251_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_252, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(252, s))
#define MSGPACK_PP_WHILE_252_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_253, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(253, s))
#define MSGPACK_PP_WHILE_253_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_254, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(254, s))
#define MSGPACK_PP_WHILE_254_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_255, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(255, s))
#define MSGPACK_PP_WHILE_255_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_256, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(256, s))
#define MSGPACK_PP_WHILE_256_C(c, p, o, s)                                                                             \
    MSGPACK_PP_IIF(c, MSGPACK_PP_WHILE_257, s MSGPACK_PP_TUPLE_EAT_3)                                                  \
    (p, o, MSGPACK_PP_IIF(c, o, MSGPACK_PP_NIL MSGPACK_PP_TUPLE_EAT_2)(257, s))
#
#
#endif