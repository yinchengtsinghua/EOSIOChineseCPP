
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/***************************************************************
 *
 *本软件的作者是David M.Gay。
 *
 *版权所有（c）1991、2000、2001朗讯科技。
 *
 *使用、复制、修改和分发本软件的权限
 *特此授予无需付费的目的，前提是整个通知
 *包含在任何软件的所有副本中，该软件是或包括一个副本。
 *或修改本软件及所有支持文件的副本。
 *此类软件的文档。
 *
 *本软件按“原样”提供，无任何明示或暗示。
 ＊保证。尤其是，无论是作者还是朗讯
 *关于适销性的任何形式的陈述或保证
 *本软件或其适用于任何特定用途。
 *
 **********************************************************/


/*请将错误报告发送给david m.gay（dmg at acm dot org，
 *将“at”更改为“@”，“dot”更改为“.”）。*/


/*在具有IEEE扩展精度寄存器的机器上，
 *需要指定双精度（53位）舍入精度
 *在调用strtod或dtoa之前。如果机器使用
 *of）Intel 80x87算术，调用
 *控制87（PC_53，MCW_PC）；
 *在许多编译器中都可以这样做。不管这个还是另一个电话
 *适当取决于编译器；要使其工作，可能需要
 *必须包含“float.h”或其他与系统相关的头段
 *文件。
 **/


/*用于IEEE、VAX和IBM算术机器的strtod。
 （注意，IEEE算法被gcc的-ffast数学标志禁用。）
 *
 *此strtod返回最接近输入小数的机器号。
 *字符串（或将errno设置为erange）。对于IEEE算法，TIE是
 *被IEEE均衡规则打破。否则领带会被
 *偏四舍五入（加半并切掉）。
 *
 *灵感来源于威廉D.克林格的论文“如何阅读浮动”
 *点号准确“[程序ACM Sigplan'90，第92-101页]。
 *
 *修改：
 *
 * 1。我们只需要IEEE、IBM或VAX双精度
 *算术（不是IEEE双扩展）。
 * 2。在这种情况下，我们通过浮点运算
 *克林格错过了——当我们计算d*10^n时
 *对于小整数d，整数n不是太大。
 *远大于22（最大整数k
 *我们可以精确代表10^k），我们可以
 *计算（d*10^k）*10^（e-k），只需一个舍入。
 * 3。而不是一点一点地调整二进制
 *在硬情况下，我们使用浮点
 *确定调整范围的算法
 *一点；只有在非常困难的情况下，我们才需要
 *计算第二个余数。
 * 4。因为3，我们不需要一个大的10次幂表
 *对于10到E（只有一些小桌子，例如10^K的桌子）
 *对于0<=k<=22）。
 **/


/*
 *为IEEE算术机器定义IEEE8087，其中
 *有效字节的地址最低。
 *为IEEE算术机器定义ieee_mc68k，其中
 *有效字节的地址最低。
 *在32位int和64位long的机器上定义long int。
 *为IBM大型机类型的浮点运算定义IBM。
 *为VAX类型的浮点运算（D_浮点）定义VAX。
 *定义no-left right以忽略快速浮点中的左右逻辑
 *DTOA的计算。这将导致DTOA模式4和5
 *对于某些输入，与模式2和模式3相同。
 *如果flt诳u rounds可以假定值为2或3，则定义荣誉诳flt诳u rounds
 *strtod和dtoa应相应地四舍五入。除非信任你
 *也定义了，将查询fegetRound（）的舍入模式。
 *请注意，flt_rounds和fegetround（）都由c99指定。
 *标准（并规定与fesetround（）一致）
 *影响flt-rounds的值），但一些（linux）系统
 *在这方面不能正常工作，因此使用fegetRound（）更重要。
 *比直接使用flt_子弹更方便携带。
 *如果flt诳u rounds可以假定值为2或3，则定义check诳flt诳u rounds
 *荣誉回合没有定义。
 *定义RND诳U ProdQuot以使用RND诳U Prod和RND诳U Quot（装配例程
 *使用扩展精度指令计算四舍五入
 *产品和报价）。
 *使用有偏差的舍入和算术为IEEE格式定义有偏差的舍入
 *向+无穷大的方向旋转。
 *定义带偏差的IEEE格式的不带偏差的round
 *当基础浮点运算使用
 *无偏舍入。这样可以防止使用普通浮点
 *计算结果有一个舍入误差的算术。
 *为IEEE格式定义不准确的除法，正确取整
 *产品，但报价不准确，例如英特尔i860。
 *在没有“长-长”的机器上定义“不长-长”
 *整数类型（大于等于64位）。在这种机器上，你可以
 *定义为执行时每32位长度存储16位
 *高精度整数运算。这是否会加快速度
 *提高或降低速度取决于机器和数量。
 *正在转换。如果long long可用并且名称为
 *除“long long long”外，还定义llong为名称，
 *如果“unsigned llong”不能作为
 *llong，将ullong定义为对应的无符号类型。
 *为旧式C函数头定义kr_头。
 *如果您的系统缺少float.h或没有，请定义bad诳float。
 *定义部分或全部dbl_dig、dbl_max_10_exp、dbl_max_exp，
 *flt_基数、flt_轮数和dbl_max。
 *定义malloc您的malloc，其中您的malloc（n）的行为类似malloc（n）
 *如果内存可用，或者您认为是
 *适当。如果未定义malloc，将调用malloc
 *直接——并假定总是成功。同样，如果你
 *希望调用系统的free（）以外的内容
 *回收从malloc获得的内存，将free定义为
 *备用程序的名称。（只调用free或free
 *病理情况，例如，在DTOA返回后的DTOA呼叫中
 *模式3要求数千位。）
 *定义omit-private-memory以省略逻辑（1998年1月添加），用于
 *尽可能从专用内存池分配内存。
 *使用时，专用池为专用内存字节长：2304字节，
 *除非定义为不同的长度。此默认长度
 *足以消除malloc呼叫，但异常情况除外，
 *例如十进制到二进制转换一个非常长的字符串
 *数字。可以返回的最长字符串DTOA大约为751字节
 *长。用于按strtod转换800位字符串和
 *8字节单线程执行中的所有DTOA转换
 *指针，private_mem>=7400似乎足够；4字节
 *指针，private_mem>=7112显示足够。
 *如果您不希望进行infnan检查，请定义no infnan检查
 *在IEEE系统上自动定义。在这样的系统上，
 *当定义了infnan_检查时，strtod检查
 *对于无穷大和NaN（不敏感的大小写）。关于某些系统
 （例如，某些HP系统），可能需要定义Nan字0
 *恰如其分——用一个安静的南族人最重要的话来说。
 *（在HP 700/800系列机器上，-dnan_Word0=0x7ff40000工作。）
 *当定义了infnan_检查且未定义任何十六进制时，
 *strtod还接受表单的（不区分大小写）字符串
 *NaN（x），其中x是十六进制数字和空格的字符串；
 *如果只有一个十六进制数字串，则取其值。
 *对于得到的NaN的52个分数位；如果有两个
 *或更多十六进制数字串，第一个是高20位，
 *低32位的第二个和后续的，中间
 *忽略空白；但如果这导致52个
 *分数位处于开启状态（一个IEEE无穷大符号），然后是Nan word0
 *使用Nan_Word1代替。
 *如果系统提供预先计划的线程，则定义多个线程
 *多线程。在这种情况下，您必须提供（或适当地
 *定义两个锁，由Acquire-DTOA-Lock（n）获取并释放
 *对于n=0或1，通过自由锁（n）。（第二个锁，进入
 *在POW5MULT中，确保仅对一个高
 *5的幂；省略此锁将导致
 *浪费记忆的可能性，否则是无害的。）
 *您还必须调用freedtoa（s）来释放返回的值s。
 * DTOA。无论是否定义了多个线程，都可以这样做。
 *在strtod中定义no-ieee-scale来禁用新的（1997年2月）逻辑
 *避免结果不下溢的输入下溢。
 *如果您在使用IEEE格式的机器上不定义任何IEEE比例
 *浮点数并将下溢刷新为零
 *与逐渐下溢相比，您还必须定义
 *突然下溢。
 *定义使用区域设置以使用当前区域设置的小数点值。
 *如果使用IEEE算法，则定义集合不精确，并且额外
 *当
 *结果不准确，避免在结果
 *是精确的。在这种情况下，必须在
 *环境，可能由提供，包括“dtoa.c”在
 *合适的包装器，定义了两个函数，
 *int不精确（无效）；
 *作废清除（作废）；
 *这样，如果
 *已经设置了不精确位，而clear\unexact（）将设置
 *不精确位为0。当set_incact被定义时，strtod
 *还进行额外计算以设置下溢和溢出
 *适当时标记（即，当结果很小且
 *不精确或是一个四舍五入为+无穷大的数值）。
 *如果strtod不应将errno=errange分配给
 *结果溢出到+无穷大或下溢到0。
 *不定义十六进制浮点以忽略十六进制浮点的识别
 *按strtod的值。
 *不定义&strtod&u bigcomp（目前仅在IEEE算术系统上）
 *禁用“快速”测试超长输入字符串的逻辑
 *转向。此测试首先截断
 *输入字符串，然后根据需要将整个字符串与
 *小数展开以决定结束大小写。这个逻辑只是
 *用于输入超过strtod_diglim位长的数字（默认为40）。
 **/


#ifndef Long
#define Long long
#endif
#ifndef ULong
typedef unsigned Long ULong;
#endif

#ifdef DEBUG
#include "stdio.h"
#define Bug(x) {fprintf(stderr, "%s\n", x); exit(1);}
#endif

#include "stdlib.h"
#include "string.h"

#ifdef USE_LOCALE
#include "locale.h"
#endif

#ifdef Honor_FLT_ROUNDS
#ifndef Trust_FLT_ROUNDS
#include <fenv.h>
#endif
#endif

#ifdef MALLOC
#ifdef KR_headers
extern char *MALLOC();
#else
extern void *MALLOC(size_t);
#endif
#else
#define MALLOC malloc
#endif

#ifndef Omit_Private_Memory
#ifndef PRIVATE_MEM
#define PRIVATE_MEM 2304
#endif
#define PRIVATE_mem ((PRIVATE_MEM+sizeof(double)-1)/sizeof(double))
static double private_mem[PRIVATE_mem], *pmem_next = private_mem;
#endif

#undef IEEE_Arith
#undef Avoid_Underflow
#ifdef IEEE_MC68k
#define IEEE_Arith
#endif
#ifdef IEEE_8087
#define IEEE_Arith
#endif

#ifdef IEEE_Arith
#ifndef NO_INFNAN_CHECK
#undef INFNAN_CHECK
#define INFNAN_CHECK
#endif
#else
#undef INFNAN_CHECK
#define NO_STRTOD_BIGCOMP
#endif

#include "errno.h"

#ifdef Bad_float_h

#ifdef IEEE_Arith
#define DBL_DIG 15
#define DBL_MAX_10_EXP 308
#define DBL_MAX_EXP 1024
#define FLT_RADIX 2
/*dif/*ieee算术

IBM公司
定义dbl_dig 16
定义dbl_max_10_exp 75
定义dbl_max_exp 63
定义flt_基数16
定义dbl_max 7.2370055773322621e+75
第二节

ViFF VAX
定义dbl_dig 16
定义dbl_max_10_exp 38
定义dbl_max_exp 127
定义flt_基数2
定义dbl_max 1.7014118346046923e+38
第二节

ifndef long_u max
定义long覕max 2147483647
第二节

else/*如果发生故障，浮动*/

#include "float.h"
/*DIF/*错误的浮点数

ifndef_uu math_u h_u
包括“math.h”
第二节

ifdef_uu cplusplus
外部“C”{
第二节

αiIFNDEF常数
ifdef kr_头
定义常量/*空白*/

#else
#define CONST const
#endif
#endif

#if defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1
Exactly one of IEEE_8087, IEEE_MC68k, VAX, or IBM should be defined.
#endif

typedef union { double d; ULong L[2]; } U;

#ifdef IEEE_8087
#define word0(x) (x)->L[1]
#define word1(x) (x)->L[0]
#else
#define word0(x) (x)->L[0]
#define word1(x) (x)->L[1]
#endif
#define dval(x) (x)->d

#ifndef STRTOD_DIGLIM
#define STRTOD_DIGLIM 40
#endif

#ifdef DIGLIM_DEBUG
extern int strtod_diglim;
#else
#define strtod_diglim STRTOD_DIGLIM
#endif

/*下面的storeinc定义适用于MIPS处理器。
 *在某些机器上可能更好的选择是
 *定义storeinc（a，b，c）（*a++=b<<16 c&0xffff）
 **/

#if defined(IEEE_8087) + defined(VAX)
#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b, \
((unsigned short *)a)[0] = (unsigned short)c, a++)
#else
#define Storeinc(a,b,c) (((unsigned short *)a)[0] = (unsigned short)b, \
((unsigned short *)a)[1] = (unsigned short)c, a++)
#endif

/*定义p dbl_mant_dig*/
/*Ten_Pmax=楼层（P*对数（2）/对数（5））*/
/*Bletch=（2的最大功率<dbl_max_10_exp）/16*/
/*quick_max=楼层（（P-1）*对数（flt_基数）/对数（10）-1）*/
/*int_max=楼层（p*log（flt_基数）/log（10）-1）*/

#ifdef IEEE_Arith
#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Nbits 53
#define Bias 1023
#define Emax 1023
#define Emin (-1022)
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#ifndef NO_IEEE_Scale
#define Avoid_Underflow
/*def flush_denorm/*调试选项*/
突然下溢
第二节
第二节

ifndef flt_轮
ifdef flt_轮
定义FLT轮数
否则
定义FLTU第1轮
第二节
endif/*flt_轮*/


#ifdef Honor_FLT_ROUNDS
#undef Check_FLT_ROUNDS
#define Check_FLT_ROUNDS
#else
#define Rounding Flt_Rounds
#endif

/*SE/*ifndef IEEE算术*/
取消定义检查
不死的荣誉
不死族集合不精确
突然下溢
定义突然下溢
IBM公司
未变形飞行物
定义FLTU第0轮
定义exp_shift 24
定义exp_shift1 24
定义exp诳msk1 0x1000000
定义exp诳msk11 0x10000000
定义exp_掩码0x7f000000
定义p 14
定义NBITS 56
定义拜厄斯65
定义EMAX 248
定义emin（-260）
定义exp_1 0x41000000
定义exp_11 0x41000000
define ebits 8/*指数有7位，但8是B2D中的正确值*/

#define Frac_mask  0xffffff
#define Frac_mask1 0xffffff
#define Bletch 4
#define Ten_pmax 22
#define Bndry_mask  0xefffff
#define Bndry_mask1 0xffffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 4
#define Tiny0 0x100000
#define Tiny1 0
#define Quick_max 14
#define Int_max 15
/*SE/*VAX*/
未变形飞行物
定义FLTU第1轮
定义exp_shift 23
定义exp移动17
定义exp诳msk1 0x80
定义exp诳msk11 0x800000
定义exp_mask 0x7f80
定义p 56
定义NBITS 56
定义偏差129
定义EMAX 126
定义emin（-129）
定义exp_1 0x408000000
定义exp_11 0x4080
定义EBIT 8
定义frac_mask 0x7ffffff
定义frac_mask1 0xffff007f
定义10诳pmax 24
定义出血2
定义bndry_mask 0xffff007f
定义bndry_mask1 0xffff007f
定义LSB 0x1000
定义符号位0x8000
定义Log2p 1
定义tiny0 0x80
定义TiNY1 0
定义“快速”最大值15
定义int_max 15
endif/*IBM、VAX*/

/*dif/*ieee算术

ifndef-ieee-arith
定义圆偏压
否则
ifdef round诳u偏向诳u而没有诳u round诳u
不死之轮_偏向
定义圆偏压
第二节
第二节

IFDEF RNDU产品
定义四舍五入的产品（a，b）a=rnd_prod（a，b）
定义四舍五入商（a，b）a=rnd_quot（a，b）
ifdef kr_头
extern double rnd_prod（），rnd_quot（）；
否则
外部双RND生产（双，双），RND配额（双，双）；
第二节
否则
定义四舍五入的产品（a，b）a*=b
定义四舍五入商（a，b）a/=b
第二节

定义big0
定义big1 0xffffffff

FiNDEF PACK32
定义包32
第二节

typedef结构bcinfo bcinfo；
 结构
bcinfo int dp0，dp1，dplen，dsign，e0，incact，nd，nd0，rounding，scale，uflchk；

ifdef kr_头
定义ffffffff（（（无符号长）0xffff）<<16）（无符号长）0xffff）
否则
定义ffffffff 0xffffffffful
第二节

ifdef不长
超长时间
γIFIFF贾斯汀16
包装盒32
/*未定义pack_32时，我们每32位长度存储16位。
 *这使得一些内部循环更简单，有时可以节省工作
 *在乘法过程中，但似乎经常会使事情稍微复杂一些。
 *速度较慢。因此，现在的默认值是存储32位/长。
 **/

#endif
/*SE/*长款可选*/
L·IFNDEF LLon
定义llong long long long
第二节
乌龙德隆
定义ullong unsigned llong
第二节
endif/*不长*/


#ifndef MULTIPLE_THREADS
/*很好地获取锁（n）/*无*/
定义自由锁（n）/*无任何内容*/

#endif

#define Kmax 7

#ifdef __cplusplus
extern "C" double strtod(const char *s00, char **se);
extern "C" char *dtoa(double d, int mode, int ndigits,
			int *decpt, int *sign, char **rve);
#endif

 struct
Bigint {
	struct Bigint *next;
	int k, maxwds, sign, wds;
	ULong x[1];
	};

 typedef struct Bigint Bigint;

 static Bigint *freelist[Kmax+1];

 static Bigint *
Balloc
#ifdef KR_headers
	(k) int k;
#else
	(int k)
#endif
{
	int x;
	Bigint *rv;
#ifndef Omit_Private_Memory
	unsigned int len;
#endif

	ACQUIRE_DTOA_LOCK(0);
 /*k>kmax案例不需要获取锁（0），*/
 /*但这种情况似乎不太可能。*/
	if (k <= Kmax && (rv = freelist[k]))
		freelist[k] = rv->next;
	else {
		x = 1 << k;
#ifdef Omit_Private_Memory
		rv = (Bigint *)MALLOC(sizeof(Bigint) + (x-1)*sizeof(ULong));
#else
		len = (sizeof(Bigint) + (x-1)*sizeof(ULong) + sizeof(double) - 1)
			/sizeof(double);
		if (k <= Kmax && pmem_next - private_mem + len <= PRIVATE_mem) {
			rv = (Bigint*)pmem_next;
			pmem_next += len;
			}
		else
			rv = (Bigint*)MALLOC(len*sizeof(double));
#endif
		rv->k = k;
		rv->maxwds = x;
		}
	FREE_DTOA_LOCK(0);
	rv->sign = rv->wds = 0;
	return rv;
	}

 static void
Bfree
#ifdef KR_headers
	(v) Bigint *v;
#else
	(Bigint *v)
#endif
{
	if (v) {
		if (v->k > Kmax)
#ifdef FREE
			FREE((void*)v);
#else
			free((void*)v);
#endif
		else {
			ACQUIRE_DTOA_LOCK(0);
			v->next = freelist[v->k];
			freelist[v->k] = v;
			FREE_DTOA_LOCK(0);
			}
		}
	}

#define Bcopy(x,y) memcpy((char *)&x->sign, (char *)&y->sign, \
y->wds*sizeof(Long) + 2*sizeof(int))

 static Bigint *
multadd
#ifdef KR_headers
	(b, m, a) Bigint *b; int m, a;
#else
 /*gint*b，int m，int a）/*乘以m，再加上a*/
第二节
{
 IN，WDS；
乌龙
 ULU*X；
 乌龙·卡里，Y；
否则
 ulong carry，*x，y；
FiDEF PACK32
 乌龙溪，Z；
第二节
第二节
 BigIt*B1；

 WDS= B-> WDS；
 X= B-> X；
 i＝0；
 进位＝A；
 做{
乌龙
  y=*x*（ullong）m+进位；
  进位=Y>>32；
  *x++=y&ffffffff；
否则
FiDEF PACK32
  XI= *X；
  y＝（Xi和0xFFFF）*M+进位；
  Z=（Xi＞16）*M+（Y>＞16）；
  进位=Z>>16；
  *x++=（z<16）+（y&0xffff）；
否则
  y=*x*m+进位；
  进位=Y>>16；
  *x++=y&0xffff；
第二节
第二节
  }
  当（++i<wds）；
 如果（进位）{
  如果（wds>=b->maxwds）
   b1=气球（b->k+1）；
   Bcopy（B1，B）；
   Bfree（b）；
   B= B1；
   }
  b->x[wds++]=进位；
  B-> WDS= WDS；
  }
 返回B；
 }

 静态双关键字
S2B
ifdef kr_头
 （s，nd0，nd，y9，dplen）const char*s；int nd0，nd，dplen；ulong y9；
否则
 （const char*s，int nd0，int nd，ulong y9，int dplen）
第二节
{
 BigIt*B；
 int i，k；
 长X，Y；

 x=（nd+8）/9；
 对于（k=0，y=1；x>y；y<<=1，k++）；
FiDEF PACK32
 B=气球（K）；
 B-> X〔0〕＝Y9；
 B-> WDS＝1；
否则
 B=气球（K+1）；
 b->x[0]=y9&0xffff；
 B->Wds=（B->X[1]=Y9>>16）？2：1；
第二节

 i＝9；
 如果（9＜ND0）{
  S+＝9；
  do b=multad（b，10，*s++--0’）；
   当（++i<nd0）；
  S++DPLEN；
  }
 其他的
  S+＝DPLEN＋9；
 对于（；i<nd；i++）
  b=multad（b，10，*s++--0'）；
 返回B；
 }

 静态int
零位
ifdef kr_头
 （x）ULUX X；
否则
 （乌龙X）
第二节
{
 INT K＝0；

 如果（！）（x和0xffff0000））
  K＝16；
  x<< 16；
  }
 如果（！）（x&0xff000000））
  k+＝8；
  x<< 8；
  }
 如果（！）（x&0xf000000））
  k+＝4；
  x<< 4；
  }
 如果（！）（x&0xc000000））
  k+＝2；
  x<< 2；
  }
 如果（！）（X&0x8000000））
  K++；
  如果（！）（X和0x40000000）
   返回32；
  }
 返回K；
 }

 静态int
零位
ifdef kr_头
 （Y）ULUN * Y；
否则
 （ULU*Y）
第二节
{
 int k；
 ULUN X＝*Y；

 如果（x和7）{
  如果（x和1）
   返回0；
  如果（x和2）{
   ＊y＝x＞1；
   返回1；
   }
  ＊y＝x＞2；
  返回2；
  }
 K＝0；
 如果（！）（x＆0xfffff）{
  K＝16；
  x>＝16；
  }
 如果（！）（x＆0xFF）{
  k+＝8；
  x>＝8；
  }
 如果（！）（x＆0xf）{
  k+＝4；
  x>＝4；
  }
 如果（！）（x＆0x3）{
  k+＝2；
  x>＝2；
  }
 如果（！）（x和1）{
  K++；
  x>＝1；
  如果（！）X）
   返回32；
  }
 ＊y= x；
 返回K；
 }

 静态双关键字
I2B
ifdef kr_头
 （i）INTI；
否则
 （int i）
第二节
{
 BigIt*B；

 B= BALC（1）；
 B-> X〔0〕＝I；
 B-> WDS＝1；
 返回B；
 }

 静态双关键字
穆尔特
ifdef kr_头
 （a，b）bigint*a，*b；
否则
 （bigint*a，bigint*b）
第二节
{
 BigIt*C；
 国际k，wa，wb，wc；
 ulong*x，*xa，*xae，*xb，*xbe，*xc，*xc0；
 乌龙Y；
乌龙
 乌龙·卡里，Z；
否则
 ULUN进位，Z；
FiDEF PACK32
 乌龙Z2；
第二节
第二节

 如果（a->wds<b->wds）
  C＝A；
  A= B；
  B＝C；
  }
 K＝A-＞K；
 WA= A-＞WDS；
 WB= B-> WDS；
 WC= WA+WB；
 如果（wc>a->maxwds）
  K++；
 C＝BARC（K）；
 对于（x=c->x，xa=x+wc；x<xa；x++）
  * x＝0；
 Xa＝a-＞x；
 XAE= XA+WA；
 XB= B-> X；
 XBE＝XB+WB；
 XC0= C-> X；
乌龙
 对于（；xb<xbe；xc0++）
  如果（（y=*xb++）
   x= xa；
   XC= XC0；
   进位＝0；
   做{
    Z=*X++*（乌龙）Y+*XC+进位；
    进位=Z>>32；
    *xc++=z&ffffffff；
    }
    而（x<xAE）；
   *XC=进位；
   }
  }
否则
FiDEF PACK32
 对于（；xb<xbe；xb++，xc0++）
  如果（y=*xb&0xffff）
   x= xa；
   XC= XC0；
   进位＝0；
   做{
    Z=（*X&0xffff）*Y+（*XC&0xffff）+进位；
    进位=Z>>16；
    Z2=（*X++>>16）*Y+（*XC>>16）+进位；
    进位=Z2>>16；
    StoreInc（XC，Z2，Z）公司；
    }
    而（x<xAE）；
   *XC=进位；
   }
  如果（y=*xb>>16）
   x= xa；
   XC= XC0；
   进位＝0；
   Z2= *XC；
   做{
    Z=（*X&0xffff）*Y+（*XC>>16）+进位；
    进位=Z>>16；
    StoreInc（XC、Z、Z2）；
    z2=（*x++>>16）*y+（*xc&0xffff）+进位；
    进位=Z2>>16；
    }
    而（x<xAE）；
   *XC= Z2；
   }
  }
否则
 对于（；xb<xbe；xc0++）
  如果（y=*xb++）
   x= xa；
   XC= XC0；
   进位＝0；
   做{
    Z=*X++*Y+*XC+进位；
    进位=Z>>16；
    *xc++=z&0xffff；
    }
    而（x<xAE）；
   *XC=进位；
   }
  }
第二节
第二节
 对于（x c 0=c->x，x c=x c 0+wc；wc>0&！*-XC；-WC）；
 C＞WDS= WC；
 返回C；
 }

 静态Bigint*P5S；

 静态双关键字
POW5Mult
ifdef kr_头
 （b，k）bigint*b；int k；
否则
 （bigint*b，int k）
第二节
{
 Bigint*b1，*p5，*p51；
 INTI；
 静态int p05[3]=5，25，125

 如果（i=k&3）
  b=multad（b，p05[i-1]，0）；

 如果（！）（k>＞2）
  返回B；
 如果（！）（p5＝p5s）{
  第一次*/

#ifdef MULTIPLE_THREADS
		ACQUIRE_DTOA_LOCK(1);
		if (!(p5 = p5s)) {
			p5 = p5s = i2b(625);
			p5->next = 0;
			}
		FREE_DTOA_LOCK(1);
#else
		p5 = p5s = i2b(625);
		p5->next = 0;
#endif
		}
	for(;;) {
		if (k & 1) {
			b1 = mult(b, p5);
			Bfree(b);
			b = b1;
			}
		if (!(k >>= 1))
			break;
		if (!(p51 = p5->next)) {
#ifdef MULTIPLE_THREADS
			ACQUIRE_DTOA_LOCK(1);
			if (!(p51 = p5->next)) {
				p51 = p5->next = mult(p5,p5);
				p51->next = 0;
				}
			FREE_DTOA_LOCK(1);
#else
			p51 = p5->next = mult(p5,p5);
			p51->next = 0;
#endif
			}
		p5 = p51;
		}
	return b;
	}

 static Bigint *
lshift
#ifdef KR_headers
	(b, k) Bigint *b; int k;
#else
	(Bigint *b, int k)
#endif
{
	int i, k1, n, n1;
	Bigint *b1;
	ULong *x, *x1, *xe, z;

#ifdef Pack_32
	n = k >> 5;
#else
	n = k >> 4;
#endif
	k1 = b->k;
	n1 = n + b->wds + 1;
	for(i = b->maxwds; n1 > i; i <<= 1)
		k1++;
	b1 = Balloc(k1);
	x1 = b1->x;
	for(i = 0; i < n; i++)
		*x1++ = 0;
	x = b->x;
	xe = x + b->wds;
#ifdef Pack_32
	if (k &= 0x1f) {
		k1 = 32 - k;
		z = 0;
		do {
			*x1++ = *x << k | z;
			z = *x++ >> k1;
			}
			while(x < xe);
		if ((*x1 = z))
			++n1;
		}
#else
	if (k &= 0xf) {
		k1 = 16 - k;
		z = 0;
		do {
			*x1++ = *x << k  & 0xffff | z;
			z = *x++ >> k1;
			}
			while(x < xe);
		if (*x1 = z)
			++n1;
		}
#endif
	else do
		*x1++ = *x++;
		while(x < xe);
	b1->wds = n1 - 1;
	Bfree(b);
	return b1;
	}

 static int
cmp
#ifdef KR_headers
	(a, b) Bigint *a, *b;
#else
	(Bigint *a, Bigint *b)
#endif
{
	ULong *xa, *xa0, *xb, *xb0;
	int i, j;

	i = a->wds;
	j = b->wds;
#ifdef DEBUG
	if (i > 1 && !a->x[i-1])
		Bug("cmp called with a->x[a->wds-1] == 0");
	if (j > 1 && !b->x[j-1])
		Bug("cmp called with b->x[b->wds-1] == 0");
#endif
	if (i -= j)
		return i;
	xa0 = a->x;
	xa = xa0 + j;
	xb0 = b->x;
	xb = xb0 + j;
	for(;;) {
		if (*--xa != *--xb)
			return *xa < *xb ? -1 : 1;
		if (xa <= xa0)
			break;
		}
	return 0;
	}

 static Bigint *
diff
#ifdef KR_headers
	(a, b) Bigint *a, *b;
#else
	(Bigint *a, Bigint *b)
#endif
{
	Bigint *c;
	int i, wa, wb;
	ULong *xa, *xae, *xb, *xbe, *xc;
#ifdef ULLong
	ULLong borrow, y;
#else
	ULong borrow, y;
#ifdef Pack_32
	ULong z;
#endif
#endif

	i = cmp(a,b);
	if (!i) {
		c = Balloc(0);
		c->wds = 1;
		c->x[0] = 0;
		return c;
		}
	if (i < 0) {
		c = a;
		a = b;
		b = c;
		i = 1;
		}
	else
		i = 0;
	c = Balloc(a->k);
	c->sign = i;
	wa = a->wds;
	xa = a->x;
	xae = xa + wa;
	wb = b->wds;
	xb = b->x;
	xbe = xb + wb;
	xc = c->x;
	borrow = 0;
#ifdef ULLong
	do {
		y = (ULLong)*xa++ - *xb++ - borrow;
		borrow = y >> 32 & (ULong)1;
		*xc++ = y & FFFFFFFF;
		}
		while(xb < xbe);
	while(xa < xae) {
		y = *xa++ - borrow;
		borrow = y >> 32 & (ULong)1;
		*xc++ = y & FFFFFFFF;
		}
#else
#ifdef Pack_32
	do {
		y = (*xa & 0xffff) - (*xb & 0xffff) - borrow;
		borrow = (y & 0x10000) >> 16;
		z = (*xa++ >> 16) - (*xb++ >> 16) - borrow;
		borrow = (z & 0x10000) >> 16;
		Storeinc(xc, z, y);
		}
		while(xb < xbe);
	while(xa < xae) {
		y = (*xa & 0xffff) - borrow;
		borrow = (y & 0x10000) >> 16;
		z = (*xa++ >> 16) - borrow;
		borrow = (z & 0x10000) >> 16;
		Storeinc(xc, z, y);
		}
#else
	do {
		y = *xa++ - *xb++ - borrow;
		borrow = (y & 0x10000) >> 16;
		*xc++ = y & 0xffff;
		}
		while(xb < xbe);
	while(xa < xae) {
		y = *xa++ - borrow;
		borrow = (y & 0x10000) >> 16;
		*xc++ = y & 0xffff;
		}
#endif
#endif
	while(!*--xc)
		wa--;
	c->wds = wa;
	return c;
	}

 static double
ulp
#ifdef KR_headers
	(x) U *x;
#else
	(U *x)
#endif
{
	Long L;
	U u;

	L = (word0(x) & Exp_mask) - (P-1)*Exp_msk1;
#ifndef Avoid_Underflow
#ifndef Sudden_Underflow
	if (L > 0) {
#endif
#endif
#ifdef IBM
		L |= Exp_msk1 >> 4;
#endif
		word0(&u) = L;
		word1(&u) = 0;
#ifndef Avoid_Underflow
#ifndef Sudden_Underflow
		}
	else {
		L = -L >> Exp_shift;
		if (L < Exp_shift) {
			word0(&u) = 0x80000 >> L;
			word1(&u) = 0;
			}
		else {
			word0(&u) = 0;
			L -= Exp_shift;
			word1(&u) = L >= 31 ? 1 : 1 << 31 - L;
			}
		}
#endif
#endif
	return dval(&u);
	}

 static double
b2d
#ifdef KR_headers
	(a, e) Bigint *a; int *e;
#else
	(Bigint *a, int *e)
#endif
{
	ULong *xa, *xa0, w, y, z;
	int k;
	U d;
#ifdef VAX
	ULong d0, d1;
#else
#define d0 word0(&d)
#define d1 word1(&d)
#endif

	xa0 = a->x;
	xa = xa0 + a->wds;
	y = *--xa;
#ifdef DEBUG
	if (!y) Bug("zero y in b2d");
#endif
	k = hi0bits(y);
	*e = 32 - k;
#ifdef Pack_32
	if (k < Ebits) {
		d0 = Exp_1 | y >> (Ebits - k);
		w = xa > xa0 ? *--xa : 0;
		d1 = y << ((32-Ebits) + k) | w >> (Ebits - k);
		goto ret_d;
		}
	z = xa > xa0 ? *--xa : 0;
	if (k -= Ebits) {
		d0 = Exp_1 | y << k | z >> (32 - k);
		y = xa > xa0 ? *--xa : 0;
		d1 = z << k | y >> (32 - k);
		}
	else {
		d0 = Exp_1 | y;
		d1 = z;
		}
#else
	if (k < Ebits + 16) {
		z = xa > xa0 ? *--xa : 0;
		d0 = Exp_1 | y << k - Ebits | z >> Ebits + 16 - k;
		w = xa > xa0 ? *--xa : 0;
		y = xa > xa0 ? *--xa : 0;
		d1 = z << k + 16 - Ebits | w << k - Ebits | y >> 16 + Ebits - k;
		goto ret_d;
		}
	z = xa > xa0 ? *--xa : 0;
	w = xa > xa0 ? *--xa : 0;
	k -= Ebits + 16;
	d0 = Exp_1 | y << k + 16 | z << k | w >> 16 - k;
	y = xa > xa0 ? *--xa : 0;
	d1 = w << k + 16 | y << k;
#endif
 ret_d:
#ifdef VAX
	word0(&d) = d0 >> 16 | d0 << 16;
	word1(&d) = d1 >> 16 | d1 << 16;
#else
#undef d0
#undef d1
#endif
	return dval(&d);
	}

 static Bigint *
d2b
#ifdef KR_headers
	(d, e, bits) U *d; int *e, *bits;
#else
	(U *d, int *e, int *bits)
#endif
{
	Bigint *b;
	int de, k;
	ULong *x, y, z;
#ifndef Sudden_Underflow
	int i;
#endif
#ifdef VAX
	ULong d0, d1;
	d0 = word0(d) >> 16 | word0(d) << 16;
	d1 = word1(d) >> 16 | word1(d) << 16;
#else
#define d0 word0(d)
#define d1 word1(d)
#endif

#ifdef Pack_32
	b = Balloc(1);
#else
	b = Balloc(2);
#endif
	x = b->x;

	z = d0 & Frac_mask;
 /*&=0x7fffffff；/*清除符号位，我们忽略它*/
ifdef突然下溢
 de=（int）（d0>>exp_shift）；
IBM公司
 Z＝ExpXMsK11；
第二节
否则
 如果（（de=（int）（d0>>exp_shift）））
  Z＝ExpXMGSK1；
第二节
FiDEF PACK32
 如果（（y= d1））{
  如果（（k=lo0bits（&y）））；
   x[0]=y z<（32-k）；
   Z>
   
  其他的
   x〔0〕＝y；
如果突然下溢
  我=
第二节
      b->wds=（x[1]=z）？2：1；
  }
 否则{
  k=Lo0bits（&z）；
  X〔0〕＝Z；
如果突然下溢
  我=
第二节
      B-> WDS＝1；
  k+＝32；
  }
否则
 如果（y= d1）{
  如果（k=Lo0Bits（&y））。
   如果（k>＝16）{
    x[0]=y z<<32-k&0xffff；
    X[1]=Z>>K-16&0xffff；
    x〔2〕＝Z＞K；
    i＝2；
    }
   否则{
    x[0]=y&0xffff；
    x[1]=y>>16 z<<16-k&0xffff；
    x[2]=z>>k&0xffff；
    X[3]=Z>>K+16；
    i＝3；
    }
  否则{
   x[0]=y&0xffff；
   x〔1〕＝y＞16；
   x[2]=z&0xffff；
   x〔3〕＝Z>＞16；
   i＝3；
   }
  }
 否则{
μIFDEF调试
  如果（！）Z）
   错误（“零传递到d2b”）；
第二节
  k=Lo0bits（&z）；
  如果（k>＝16）{
   X〔0〕＝Z；
   i＝0；
   }
  否则{
   x[0]=z&0xffff；
   x〔1〕＝Z>＞16；
   i＝1；
   }
  k+＝32；
  }
 而（！）x[i]
  ——我；
 B-> WDS= I+ 1；
第二节
如果突然下溢
 如果（de）{
第二节
IBM公司
  *e=（de-偏差-（p-1）<<2）+k；
  *位=4*p+8-k-hi0bits（word0（d）&frac_mask）；
否则
  *e=去偏压（p-1）+k；
  *位＝P- K；
第二节
如果突然下溢
  }
 否则{
  *e=去偏压（p-1）+1+k；
FiDEF PACK32
  *位=32*i-hi0bits（x[i-1]）；
否则
  *位=（i+2）*16-hi0bits（x[i]）；
第二节
  }
第二节
 返回B；
 }
D0
D1

 静态双
比率
ifdef kr_头
 （a，b）bigint*a，*b；
否则
 （bigint*a，bigint*b）
第二节
{
 u，dB；
 INK，KA，KB；

 dval（&da）=b2d（a，&ka）；
 dval（&db）=b2d（b，&kb）；
FiDEF PACK32
 k=k a-k b+32*（a->wds-b->wds）；
否则
 k=k a-k b+16*（a->wds-b->wds）；
第二节
IBM公司
 如果（k＞0）{
  单词0（&da）+=（k>>2）*expmsk1；
  如果（k&＝3）
   dval（&da）*=1<<k；
  }
 否则{
  K＝-K；
  单词0（&db）+=（k>>2）*expmsk1；
  如果（k&＝3）
   dval（&db）*=1<<k；
  }
否则
 如果（k＞0）
  单词0（&da）+=k*exp_msk1；
 否则{
  K＝-K；
  单词0（&db）+=k*exp_msk1；
  }
第二节
 返回dval（&da）/dval（&db）；
 }

 静态常量双精度
TEN[]= {
  1e0，1e1，1e2，1e3，1e4，1e5，1e6，1e7，1e8，1e9，
  1E10、1E11、1E12、1E13、1E14、1E15、1E16、1E17、1E18、1E19，
  1E20、1E21、1E22
ViFF VAX
  ，1E23，1E24
第二节
  }；

 静态常量双精度
ifdef-ieee-arith
Bigtens[]=1E16、1E32、1E64、1E128、1E256_
静态常量双tinytens[]=1e-16、1e-32、1e-64、1e-128，
ifdef避免_下溢
  9007199254740992.*9007199254740992.E-256
  /*=2^106*1e-256*/

#else
		1e-256
#endif
		};
/*Tinytens中的2^53系数[4]帮助我们避免设置下溢*/
/*不必要地标记。它导致了一首歌和一支舞蹈在斯特托德的结尾。*/
#define Scale_Bit 0x10
#define n_bigtens 5
#else
#ifdef IBM
bigtens[] = { 1e16, 1e32, 1e64 };
static CONST double tinytens[] = { 1e-16, 1e-32, 1e-64 };
#define n_bigtens 3
#else
bigtens[] = { 1e16, 1e32 };
static CONST double tinytens[] = { 1e-16, 1e-32 };
#define n_bigtens 2
#endif
#endif

#undef Need_Hexdig
#ifdef INFNAN_CHECK
#ifndef No_Hex_NaN
#define Need_Hexdig
#endif
#endif

#ifndef Need_Hexdig
#ifndef NO_HEX_FP
#define Need_Hexdig
#endif
#endif

/*def需要_hexdig/**/
若0
静态无符号字符hexdig[256]；

 静态空隙
htinit（无符号char*h，无符号char*s，int inc）
{
 int i，j；
 对于（i=0；（j=s[i]）！= 0；i++）
  H[J]＝I+Inc；
 }

 静态空隙
hexdig_init（void）/*使用hexdig_init省略了20121220以避免*/

   /*使用多个线程时的争用条件。*/
{
#define USC (unsigned char *)
	htinit(hexdig, USC "0123456789", 0x10);
	htinit(hexdig, USC "abcdef", 0x10 + 10);
	htinit(hexdig, USC "ABCDEF", 0x10 + 10);
	}
#else
static unsigned char hexdig[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,
	0,26,27,28,29,30,31,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,26,27,28,29,30,31,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
#endif
/*DIF/*需要撘hexdig*/

ifdef infnan_检查

ifndef nan_word0
定义nan砗word0 0x7ff80000
第二节

ifndef nan_word1
定义Nan覕word1 0
第二节

 静态int
比赛
ifdef kr_头
 （sp，t）字符**sp，*t；
否则
 （常量字符**sp，常量字符*t）
第二节
{
 C，D；
 常量char*s=*sp；

 同时（（d=*t++）
  如果（（c=*++s）>='a'&&c<='z'）
   C+=“A”-“A”；
  如果（C）！= D）
   返回0；
  }
 ＊SP＝S＋1；
 返回1；
 }

如果无六角螺母
 静态空隙
己南
ifdef kr_头
 （rvp，sp）u*rvp；const char**sp；
否则
 （u*rvp，常量字符**sp）
第二节
{
 ULUN C，X〔2〕；
 常量字符；
 int c1、havedig、udx0、xshift；

 [如果]！hexdig['0']）hexdig_init（）；***/

	x[0] = x[1] = 0;
	havedig = xshift = 0;
	udx0 = 1;
	s = *sp;
 /*允许可选的初始值0x或0x*/
	while((c = *(CONST unsigned char*)(s+1)) && c <= ' ')
		++s;
	if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X'))
		s += 2;
	while((c = *(CONST unsigned char*)++s)) {
		if ((c1 = hexdig[c]))
			c  = c1 & 0xf;
		else if (c <= ' ') {
			if (udx0 && havedig) {
				udx0 = 0;
				xshift = 1;
				}
			continue;
			}
#ifdef GDTOA_NON_PEDANTIC_NANCHECK
  /*e如果（/*（*/c='）'&&havedig）
   ＊SP＝S＋1；
   断裂；
   }
  其他的
   return；/*无效格式：不更改*sp*/

#else
		else {
			do {
    /*（/*（*/c='）'）
     ＊SP＝S＋1；
     断裂；
     }
    同时（（c=*++s））；
   断裂；
   }
第二节
  HoeDigy＝1；
  如果（xSHIFT）{
   xSHIFT＝0；
   x〔0〕＝x〔1〕；
   x〔1〕＝0；
   }
  如果（UDX0）
   x[0]=（x[0]<<4）（x[1]>>28）；
  x[1]=（x[1]<<4）c；
  }
 如果（（x[0]&=0xffff）x[1]）
  word0（rvp）=exp_x[0]；
  字1（rvp）=x[1]；
  }
 }
endif/*无十六进制*/

/*DIF/*infnan_检查*/

FiDEF PACK32
定义ulbits 32
定义kshift 5
定义kmask 31
否则
定义ulbits 16
定义kshift 4
定义kmask 15
第二节

如果有的话！已定义（无已定义（荣誉回合）/**/

 static Bigint *
#ifdef KR_headers
increment(b) Bigint *b;
#else
increment(Bigint *b)
#endif
{
	ULong *x, *xe;
	Bigint *b1;

	x = b->x;
	xe = x + b->wds;
	do {
		if (*x < (ULong)0xffffffffL) {
			++*x;
			return b;
			}
		*x++ = 0;
		} while(x < xe);
	{
		if (b->wds >= b->maxwds) {
			b1 = Balloc(b->k+1);
			Bcopy(b1,b);
			Bfree(b);
			b = b1;
			}
		b->x[b->wds++] = 1;
		}
	return b;
	}

/*DIF/*}*

ifndef no_hex_fp/**/


 static void
#ifdef KR_headers
rshift(b, k) Bigint *b; int k;
#else
rshift(Bigint *b, int k)
#endif
{
	ULong *x, *x1, *xe, y;
	int n;

	x = x1 = b->x;
	n = k >> kshift;
	if (n < b->wds) {
		xe = x + b->wds;
		x += n;
		if (k &= kmask) {
			n = 32 - k;
			y = *x++ >> k;
			while(x < xe) {
				*x1++ = (y | (*x << n)) & 0xffffffff;
				y = *x++ >> k;
				}
			if ((*x1 = y) !=0)
				x1++;
			}
		else
			while(x < xe)
				*x1++ = *x++;
		}
	if ((b->wds = x1 - b->x) == 0)
		b->x[0] = 0;
	}

 static ULong
#ifdef KR_headers
any_on(b, k) Bigint *b; int k;
#else
any_on(Bigint *b, int k)
#endif
{
	int n, nwds;
	ULong *x, *x0, x1, x2;

	x = b->x;
	nwds = b->wds;
	n = k >> kshift;
	if (n > nwds)
		n = nwds;
	else if (n < nwds && (k &= kmask)) {
		x1 = x2 = x[n];
		x1 >>= k;
		x1 <<= k;
		if (x1 != x2)
			return 1;
		}
	x0 = x;
	x += n;
	while(x > x0)
		if (*--x)
			return 1;
	return 0;
	}

/*m/*舍入值：与flt_rounds*相同
 圆零点＝0，
 1，
 2，
 圆向下＝3
 }；

 无效
ifdef kr_头
gethex（sp，rvp，舍入，符号）
 常量字符**sp；u*rvp；int舍入，符号；
否则
gethex（const char**sp，u*rvp，int rounding，int sign）
第二节
{
 BigIt*B；
 const unsigned char*decpt，*s0，*s，*s1；
 长e，E1；
 ulong l，lostbits，*x；
 int-big、denorm、esign、havedig、k、n、nbits、up、zret；
IBM公司
 int j；
第二节
 枚举{
ifdef-ieee35u-arith/**/

		emax = 0x7fe - Bias - P + 1,
		emin = Emin - P + 1
/*SE/*}{*/
  emin=emin-p，
ViFF VAX
  Emax=0x7FF-偏差-P+1
第二节
IBM公司
  Emax=0x7f-偏差-p
第二节
In·NeNF/*}*/

		};
#ifdef USE_LOCALE
	int i;
#ifdef NO_LOCALE_CACHE
	const unsigned char *decimalpoint = (unsigned char*)
		localeconv()->decimal_point;
#else
	const unsigned char *decimalpoint;
	static unsigned char *decimalpoint_cache;
	if (!(s0 = decimalpoint_cache)) {
		s0 = (unsigned char*)localeconv()->decimal_point;
		if ((decimalpoint_cache = (unsigned char*)
				MALLOC(strlen((CONST char*)s0) + 1))) {
			strcpy((char*)decimalpoint_cache, (CONST char*)s0);
			s0 = decimalpoint_cache;
			}
		}
	decimalpoint = s0;
#endif
#endif

 /***如果！hexdig['0']）hexdig_init（）；***/
	havedig = 0;
	s0 = *(CONST unsigned char **)sp + 2;
	while(s0[havedig] == '0')
		havedig++;
	s0 += havedig;
	s = s0;
	decpt = 0;
	zret = 0;
	e = 0;
	if (hexdig[*s])
		havedig++;
	else {
		zret = 1;
#ifdef USE_LOCALE
		for(i = 0; decimalpoint[i]; ++i) {
			if (s[i] != decimalpoint[i])
				goto pcheck;
			}
		decpt = s += i;
#else
		if (*s != '.')
			goto pcheck;
		decpt = ++s;
#endif
		if (!hexdig[*s])
			goto pcheck;
		while(*s == '0')
			s++;
		if (hexdig[*s])
			zret = 0;
		havedig = 1;
		s0 = s;
		}
	while(hexdig[*s])
		s++;
#ifdef USE_LOCALE
	if (*s == *decimalpoint && !decpt) {
		for(i = 1; decimalpoint[i]; ++i) {
			if (s[i] != decimalpoint[i])
				goto pcheck;
			}
		decpt = s += i;
#else
	if (*s == '.' && !decpt) {
		decpt = ++s;
#endif
		while(hexdig[*s])
			s++;
  /*＊＊
 如果（DEPT）
  e=-（（长）（s-decpt））<<2）；
 PCHEC:
 S1= S；
 大=设计=0；
 开关（*s）{
   案例“P”：
   案例“P”：
  开关（*+S）{
    案例“-”：
   ESGIN＝1；
   /*不中断*/

		  case '+':
			s++;
		  }
		if ((n = hexdig[*s]) == 0 || n > 0x19) {
			s = s1;
			break;
			}
		e1 = n - 0x10;
		while((n = hexdig[*++s]) !=0 && n <= 0x19) {
			if (e1 & 0xf8000000)
				big = 1;
			e1 = 10*e1 + n - 0x10;
			}
		if (esign)
			e1 = -e1;
		e += e1;
	  }
	*sp = (char*)s;
	if (!havedig)
		*sp = (char*)s0 - 1;
	if (zret)
		goto retz1;
	if (big) {
		if (esign) {
#ifdef IEEE_Arith
			switch(rounding) {
			  case Round_up:
				if (sign)
					break;
				goto ret_tiny;
			  case Round_down:
				if (!sign)
					break;
				goto ret_tiny;
			  }
#endif
			goto retz;
#ifdef IEEE_Arith
 ret_tinyf:
			Bfree(b);
 ret_tiny:
#ifndef NO_ERRNO
			errno = ERANGE;
#endif
			word0(rvp) = 0;
			word1(rvp) = 1;
			return;
/*dif/*ieee算术
   }
  开关（四舍五入）
    箱子周围：
   GOTO OVFL1；
    案例：
   如果（！）符号）
    GOTO OVFL1；
   Goto ReTyBig.
    案例取整：
   如果（符号）
    GOTO OVFL1；
   Goto ReTyBig.
    }
 瑞特比格：
  word0（rvp）=big0；
  word1（rvp）=big1；
  返回；
  }
 n=s1-s0-1；
 对于（k=0；n>（1<（kshift-2））-1；n>>=1）
  K++；
 B=气球（K）；
 X= B-> X；
 n＝0；
 L＝0；
ifdef使用区域设置
 对于（i=0；小数点[i+1]；++i）；
第二节
 同时（s1>s0）
ifdef使用区域设置
  如果（--s1==decimalpoint[i]）；
   S1-Ⅰ；
   继续；
   }
否则
  如果（*--s1=''）
   继续；
第二节
  如果（n==ulbits）
   *x++＝L；
   L＝0；
   n＝0；
   }
  L=（hexdig[*s1]&0x0f）<<n；
  n+＝4；
  }
 *x++＝L；
 b->wds=n=x-b->x；
 n=ulbits*n-hi0bits（l）；
 nBIT＝NBIT；
 LoestBIT＝0；
 X= B-> X；
 如果（n>nbits）
  n= nBIT；
  如果（b，n）上有任何_
   LoestBIT＝1；
   K＝N—1；
   if（x[k>>kshift]&1<（k&kmask））
    LoestBIT＝2；
    如果（k>0&&any_on（b，k））。
     LoestBIT＝3；
    }
   }
  RSHIFT（b，n）；
  e+n；
  }
 否则，如果（n<nbits）
  n=nB-n；
  b=lshift（b，n）；
  E＝N；
  X= B-> X；
  }
 如果（E> EMAX）{
 OVFL：
  Bfree（b）；
 OVFL1:
如果没有，请
  Erang=
第二节
  word0（rvp）=exp_掩码；
  Word1（RVP）＝0；
  返回；
  }
 DENORM＝0；
 如果（e＜Emin）{
  DENORM＝1；
  n＝EMIN -E；
  如果（n>=nbits）
ifdef-ieee-arith/**/

			switch (rounding) {
			  case Round_near:
				if (n == nbits && (n < 2 || any_on(b,n-1)))
					goto ret_tinyf;
				break;
			  case Round_up:
				if (!sign)
					goto ret_tinyf;
				break;
			  case Round_down:
				if (sign)
					goto ret_tinyf;
			  }
/*dif/*ieee_arith*/
   Bfree（b）；
 雷茨：
如果没有，请
   Erang=
第二节
 RITZ1：
   RVP＞D＝0；
   返回；
   }
  K＝N—1；
  如果（LoSTBIT）
   LoestBIT＝1；
  否则（k＞0）
   lostbits=任何打开（b，k）；
  如果（x[k>>kshift]&1<（k&kmask））
   叶位＝2；
  nBIT＝N；
  RSHIFT（b，n）；
  E = Emin；
  }
 如果（LoSTBIT）{
  UP＝0；
  开关（四舍五入）
    案例四舍五入零：
   断裂；
    箱子周围：
   如果（丢失&2
    &&（lostbits&1）（x[0]&1））
    UP＝1；
   断裂；
    案例：
   UP＝1 -符号；
   断裂；
    案例取整：
   UP =符号；
    }
  如果（up）{
   K= B-> WDS；
   b=增量（b）；
   X= B-> X；
   如果（DENORM）{
若0
    如果（nbits==nbits-1
     &&x[nbits>>kshift]&1<（nbits&kmask））
     denorm=0；/*当前未使用*/

#endif
				}
			else if (b->wds > k
			 || ((n = nbits & kmask) !=0
			     && hi0bits(x[k-1]) < 32-n)) {
				rshift(b,1);
				if (++e > Emax)
					goto ovfl;
				}
			}
		}
#ifdef IEEE_Arith
	if (denorm)
		word0(rvp) = b->wds > 1 ? b->x[1] & ~0x100000 : 0;
	else
		word0(rvp) = (b->x[1] & ~0x100000) | ((e + 0x3ff + 52) << 20);
	word1(rvp) = b->x[0];
#endif
#ifdef IBM
	if ((j = e & 3)) {
		k = b->x[0] & ((1 << j) - 1);
		rshift(b,j);
		if (k) {
			switch(rounding) {
			  case Round_up:
				if (!sign)
					increment(b);
				break;
			  case Round_down:
				if (sign)
					increment(b);
				break;
			  case Round_near:
				j = 1 << (j-1);
				if (k & j && ((k & (j-1)) | lostbits))
					increment(b);
			  }
			}
		}
	e >>= 2;
	word0(rvp) = b->x[1] | ((e + 65 + 13) << 24);
	word1(rvp) = b->x[0];
#endif
#ifdef VAX
 /*接下来的两行忽略低阶和高阶2字节的交换。*/
 /*word0（rvp）=（b->x[1]&~0x800000）（e+129+55）<<23）；*/
 /*单词1（rvp）=b->x[0]；*/
	word0(rvp) = ((b->x[1] & ~0x800000) >> 16) | ((e + 129 + 55) << 7) | (b->x[1] << 16);
	word1(rvp) = (b->x[0] >> 16) | (b->x[0] << 16);
#endif
	Bfree(b);
	}
/*dif/*！NoiHouthFP}*

 静态int
ifdef kr_头
dshift（b，p2）bigint*b；int p2；
否则
dshift（bigint*b，int p2）
第二节
{
 int rv=hi0bits（b->x[b->wds-1]）-4；
 如果（P2＞0）
  RV-＝P2；
 返回RV&kmask；
 }

 静态int
夸雷姆
ifdef kr_头
 （b，s）bigint*b，*s；
否则
 （bigint*b，bigint*s）
第二节
{
 国际标准；
 乌龙*BX，*BXE，Q，*SX，*SXE；
乌龙
 乌龙借、运、Y、Y；
否则
 乌龙借，运，Y，Y；
FiDEF PACK32
 乌龙寺、z、zs；
第二节
第二节

 n=s＞WDS；
μIFDEF调试
 /*Debug*/ if (b->wds > n)

 /*ebug*/bug（“Quorem中的超大B”）；
第二节
 如果（B-> WDS<N）
  返回0；
 SX= S-＞X；
 SXE＝SX+-N；
 BX= B-> X；
 BXE＝BX+N；
 Q=*BXE/（*SXE+1）；/*确保Q<=真商*/

#ifdef DEBUG
#ifdef NO_STRTOD_BIGCOMP
 /*ebug*/if（q>9）
否则
 /*当从bigcomp调用quorem并且*/

 /*输入接近，例如，最小非规范化数的两倍。*/
 /*ebug*/if（q>15）
第二节
 /*Debug*/	Bug("oversized quotient in quorem");

#endif
	if (q) {
		borrow = 0;
		carry = 0;
		do {
#ifdef ULLong
			ys = *sx++ * (ULLong)q + carry;
			carry = ys >> 32;
			y = *bx - (ys & FFFFFFFF) - borrow;
			borrow = y >> 32 & (ULong)1;
			*bx++ = y & FFFFFFFF;
#else
#ifdef Pack_32
			si = *sx++;
			ys = (si & 0xffff) * q + carry;
			zs = (si >> 16) * q + (ys >> 16);
			carry = zs >> 16;
			y = (*bx & 0xffff) - (ys & 0xffff) - borrow;
			borrow = (y & 0x10000) >> 16;
			z = (*bx >> 16) - (zs & 0xffff) - borrow;
			borrow = (z & 0x10000) >> 16;
			Storeinc(bx, z, y);
#else
			ys = *sx++ * q + carry;
			carry = ys >> 16;
			y = *bx - (ys & 0xffff) - borrow;
			borrow = (y & 0x10000) >> 16;
			*bx++ = y & 0xffff;
#endif
#endif
			}
			while(sx <= sxe);
		if (!*bxe) {
			bx = b->x;
			while(--bxe > bx && !*bxe)
				--n;
			b->wds = n;
			}
		}
	if (cmp(b, S) >= 0) {
		q++;
		borrow = 0;
		carry = 0;
		bx = b->x;
		sx = S->x;
		do {
#ifdef ULLong
			ys = *sx++ + carry;
			carry = ys >> 32;
			y = *bx - (ys & FFFFFFFF) - borrow;
			borrow = y >> 32 & (ULong)1;
			*bx++ = y & FFFFFFFF;
#else
#ifdef Pack_32
			si = *sx++;
			ys = (si & 0xffff) + carry;
			zs = (si >> 16) + (ys >> 16);
			carry = zs >> 16;
			y = (*bx & 0xffff) - (ys & 0xffff) - borrow;
			borrow = (y & 0x10000) >> 16;
			z = (*bx >> 16) - (zs & 0xffff) - borrow;
			borrow = (z & 0x10000) >> 16;
			Storeinc(bx, z, y);
#else
			ys = *sx++ + carry;
			carry = ys >> 16;
			y = *bx - (ys & 0xffff) - borrow;
			borrow = (y & 0x10000) >> 16;
			*bx++ = y & 0xffff;
#endif
#endif
			}
			while(sx <= sxe);
		bx = b->x;
		bxe = bx + n;
		if (!*bxe) {
			while(--bxe > bx && !*bxe)
				--n;
			b->wds = n;
			}
		}
	return q;
	}

/*定义（避免下溢）！已定义（无strtod_bigcomp）/**/
 静态双
闷热
ifdef kr_头
 （x，bc）u*x；bcinfo*bc；
否则
 （u*x，bcinfo*bc）
第二节
{
 Uü；
 双RV；
 INTI；

 RV= ULP（X）；
 如果（！）BC->scale（i=2*p+1-（（word0（x）&exp_mask）>>exp_shift））<=0）
  return rv；/*是否有i<=0的示例？*/

	word0(&u) = Exp_1 + (i << Exp_shift);
	word1(&u) = 0;
	return rv * u.d;
	}
/*DIF/*}*

如果没有strtod bigcomp
 静态空隙
大组合
ifdef kr_头
 （RV，S0，BC）
 u*rv；常量char*s0；bcinfo*bc；
否则
 （U*RV，常量char*S0，bcinfo*bc）
第二节
{
 BigIt**，*d；
 int b2、bbits、d2、dd、dig、dsign、i、j、nd、nd0、p2、p5、speccase；

 dsign=bc->dsign；
 Nd＝BC-> Nd；
 ND0＝BC-> ND0；
 p5=nd+bc->e0-1；
 ScCase= 0；
如果突然下溢
 if（rv->d==0.）/*特殊情况：下溢值接近零*/

    /*阈值四舍五入为零*/
		b = i2b(1);
		p2 = Emin - P + 1;
		bbits = 1;
#ifdef Avoid_Underflow
		word0(rv) = (P+2) << Exp_shift;
#else
		word1(rv) = 1;
#endif
		i = 0;
#ifdef Honor_FLT_ROUNDS
		if (bc->rounding == 1)
#endif
			{
			speccase = 1;
			--p2;
			dsign = 0;
			goto have_i;
			}
		}
	else
#endif
		b = d2b(rv, &p2, &bbits);
#ifdef Avoid_Underflow
	p2 -= bc->scale;
#endif
 /*地板（log2（rv））==bbits-1+p2*/
 /*检查异常情况。*/
	i = P - bbits;
	if (i > (j = P - Emin - 1 + p2)) {
#ifdef Sudden_Underflow
		Bfree(b);
		b = i2b(1);
		p2 = Emin;
		i = P - 1;
#ifdef Avoid_Underflow
		word0(rv) = (1 + bc->scale) << Exp_shift;
#else
		word0(rv) = Exp_msk1;
#endif
		word1(rv) = 0;
#else
		i = j;
#endif
		}
#ifdef Honor_FLT_ROUNDS
	if (bc->rounding != 1) {
		if (i > 0)
			b = lshift(b, i);
		if (dsign)
			b = increment(b);
		}
	else
#endif
		{
		b = lshift(b, ++i);
		b->x[0] |= 1;
		}
#ifndef Sudden_Underflow
 have_i:
#endif
	p2 -= p5 + i;
	d = i2b(1);
 /*安排方便的商计算：
  *必要时向左移位，使除数有4个前导0位。
  **/

	if (p5 > 0)
		d = pow5mult(d, p5);
	else if (p5 < 0)
		b = pow5mult(b, -p5);
	if (p2 > 0) {
		b2 = p2;
		d2 = 0;
		}
	else {
		b2 = 0;
		d2 = -p2;
		}
	i = dshift(d, d2);
	if ((b2 += i) > 0)
		b = lshift(b, b2);
	if ((d2 += i) > 0)
		d = lshift(d, d2);

 /*现在b/d=正好在两个浮点值之间的一半*/
 /*在输入字符串的任一侧。计算b/d的第一位。*/

	if (!(dig = quorem(b,d))) {
  /*multad（b，10，0）；/*非常不可能*/
  dig=quorem（b，d）；
  }

 /*将b/d与s0进行比较*/


	for(i = 0; i < nd0; ) {
		if ((dd = s0[i++] - '0' - dig))
			goto ret;
		if (!b->x[0] && b->wds == 1) {
			if (i < nd)
				dd = 1;
			goto ret;
			}
		b = multadd(b, 10, 0);
		dig = quorem(b,d);
		}
	for(j = bc->dp1; i++ < nd;) {
		if ((dd = s0[j++] - '0' - dig))
			goto ret;
		if (!b->x[0] && b->wds == 1) {
			if (i < nd)
				dd = 1;
			goto ret;
			}
		b = multadd(b, 10, 0);
		dig = quorem(b,d);
		}
	if (dig > 0 || b->x[0] || b->wds > 1)
		dd = -1;
 ret:
	Bfree(b);
	Bfree(d);
#ifdef Honor_FLT_ROUNDS
	if (bc->rounding != 1) {
		if (dd < 0) {
			if (bc->rounding == 0) {
				if (!dsign)
					goto retlow1;
				}
			else if (dsign)
				goto rethi1;
			}
		else if (dd > 0) {
			if (bc->rounding == 0) {
				if (dsign)
					goto rethi1;
				goto ret1;
				}
			if (!dsign)
				goto rethi1;
			dval(rv) += 2.*sulp(rv,bc);
			}
		else {
			bc->inexact = 0;
			if (dsign)
				goto rethi1;
			}
		}
	else
#endif
	if (speccase) {
		if (dd <= 0)
			rv->d = 0.;
		}
	else if (dd < 0) {
  /*（！dsign）/*附近的回合不发生*/
ReloLoT1:
   dval（rv）-=sulp（rv，bc）；
  }
 否则，如果（dd>0）
  如果（d签）{
 ReTi1:
   dval（rv）+=sulp（rv，bc）；
   }
  }
 否则{
  /*精确的半程情况：应用双圆规则。*/

		if ((j = ((word0(rv) & Exp_mask) >> Exp_shift) - bc->scale) <= 0) {
			i = 1 - j;
			if (i <= 31) {
				if (word1(rv) & (0x1 << i))
					goto odd;
				}
			else if (word0(rv) & (0x1 << (i-32)))
				goto odd;
			}
		else if (word1(rv) & 1) {
 odd:
			if (dsign)
				goto rethi1;
			goto retlow1;
			}
		}

#ifdef Honor_FLT_ROUNDS
 ret1:
#endif
	return;
	}
/*dif/*无strtod\u bigcomp*/

 双重的
字符串转换为浮点数
ifdef kr_头
 （S00，SE）常量字符*S00；字符**SE；
否则
 （常量char*S00，char**se）
第二节
{
 int bb2、bb5、bbe、bd2、bd5、bbbit、bs2、c、e、e1；
 内部设计，i，j，k，nd，nd0，nf，nz，nz0，nz1，符号；
 常量char*s，*s0，*s1；
 双AADJ，AADJ1；
 长L；
 u aadj2、adj、rv、rv0；
 乌龙Y，Z；
 BcFielbC；
 bigint*bb，*bb1，*bd，*bd0，*bs，*delta；
ifdef避免_下溢
 乌龙LSB，LSB1；
第二节
ifdef设置不精确
 int精确；
第二节
如果没有strtod bigcomp
 int req_bigcomp=0；
第二节
IFDEF荣誉_FLTU回合/**/

/*def trust_flt_rounds/*仅当flt_rounds真正工作时定义此项！*/
 bc.rounding=flt_轮数；
α-Tele/*}*/

	bc.rounding = 1;
	switch(fegetround()) {
	  case FE_TOWARDZERO:	bc.rounding = 0; break;
	  case FE_UPWARD:	bc.rounding = 2; break;
	  case FE_DOWNWARD:	bc.rounding = 3;
	  }
/*dif/*} */
NeNFEF/**/

#ifdef USE_LOCALE
	CONST char *s2;
#endif

	sign = nz0 = nz1 = nz = bc.dplen = bc.uflchk = 0;
	dval(&rv) = 0.;
	for(s = s00;;s++) switch(*s) {
		case '-':
			sign = 1;
   /*不中断*/
		case '+':
			if (*++s)
				goto break2;
   /*不中断*/
		case 0:
			goto ret0;
		case '\t':
		case '\n':
		case '\v':
		case '\f':
		case '\r':
		case ' ':
			continue;
		default:
			goto break2;
		}
 break2:
	if (*s == '0') {
/*ndef无十六进制
  开关（S〔1〕）{
    案例“X”：
    案例“X”：
IFDEF荣誉\U FLTU回合
   gethex（&s，&rv，bc.舍入，符号）；
否则
   gethex（&s，&rv，1，符号）；
第二节
   Goto RET；
    }
NeNFEF/**/

		nz0 = 1;
		while(*++s == '0') ;
		if (!*s)
			goto ret;
		}
	s0 = s;
	y = z = 0;
	for(nd = nf = 0; (c = *s) >= '0' && c <= '9'; nd++, s++)
		if (nd < 9)
			y = 10*y + c - '0';
		else if (nd < DBL_DIG + 2)
			z = 10*z + c - '0';
	nd0 = nd;
	bc.dp0 = bc.dp1 = s - s0;
	for(s1 = s; s1 > s0 && *--s1 == '0'; )
		++nz1;
#ifdef USE_LOCALE
	s1 = localeconv()->decimal_point;
	if (c == *s1) {
		c = '.';
		if (*++s1) {
			s2 = s;
			for(;;) {
				if (*++s2 != *s1) {
					c = 0;
					break;
					}
				if (!*++s1) {
					s = s2;
					break;
					}
				}
			}
		}
#endif
	if (c == '.') {
		c = *++s;
		bc.dp1 = s - s0;
		bc.dplen = bc.dp1 - bc.dp0;
		if (!nd) {
			for(; c == '0'; c = *++s)
				nz++;
			if (c > '0' && c <= '9') {
				bc.dp0 = s0 - s;
				bc.dp1 = bc.dp0 + bc.dplen;
				s0 = s;
				nf += nz;
				nz = 0;
				goto have_dig;
				}
			goto dig_done;
			}
		for(; c >= '0' && c <= '9'; c = *++s) {
 have_dig:
			nz++;
			if (c -= '0') {
				nf += nz;
				for(i = 1; i < nz; i++)
					if (nd++ < 9)
						y *= 10;
					else if (nd <= DBL_DIG + 2)
						z *= 10;
				if (nd++ < 9)
					y = 10*y + c;
				else if (nd <= DBL_DIG + 2)
					z = 10*z + c;
				nz = nz1 = 0;
				}
			}
		}
 dig_done:
	e = 0;
	if (c == 'e' || c == 'E') {
		if (!nd && !nz && !nz0) {
			goto ret0;
			}
		s00 = s;
		esign = 0;
		switch(c = *++s) {
			case '-':
				esign = 1;
			case '+':
				c = *++s;
			}
		if (c >= '0' && c <= '9') {
			while(c == '0')
				c = *++s;
			if (c > '0' && c <= '9') {
				L = c - '0';
				s1 = s;
				while((c = *++s) >= '0' && c <= '9')
					L = 10*L + c - '0';
				if (s - s1 > 8 || L > 19999)
     /*避免指数混淆
      *太大，E可能溢出。
      **/

     /*19999；/*16位整数安全*/
    其他的
     e=（int）L；
    如果（ESGIN）
     E＝-E；
    }
   其他的
    e＝0；
   }
  其他的
   S＝S00；
  }
 如果（！）钕）{
  如果（！）新西兰！NZ0）{
ifdef infnan_检查
   /*检查NaN和Infinity*/

			if (!bc.dplen)
			 switch(c) {
			  case 'i':
			  case 'I':
				if (match(&s,"nf")) {
					--s;
					if (!match(&s,"inity"))
						++s;
					word0(&rv) = 0x7ff00000;
					word1(&rv) = 0;
					goto ret;
					}
				break;
			  case 'n':
			  case 'N':
				if (match(&s, "an")) {
					word0(&rv) = NAN_WORD0;
					word1(&rv) = NAN_WORD1;
#ifndef No_Hex_NaN
     /*（*s='（'）/*）*/
      河川南（&rv，&s）；
第二节
     Goto RET；
     }
     }
endif/*infnan_检查*/

 ret0:
			s = s00;
			sign = 0;
			}
		goto ret;
		}
	bc.e0 = e1 = e -= nf;

 /*现在我们有了nd0位，从s0开始，然后是
  *小数点后接nd-nd0位。我们的号码
  *after是用这些数字乘以表示的整数。
  * 10×e*/


	if (!nd0)
		nd0 = nd;
	k = nd < DBL_DIG + 2 ? nd : DBL_DIG + 2;
	dval(&rv) = y;
	if (k > 9) {
#ifdef SET_INEXACT
		if (k > DBL_DIG)
			oldinexact = get_inexact();
#endif
		dval(&rv) = tens[k - 9] * dval(&rv) + z;
		}
	bd0 = 0;
	if (nd <= DBL_DIG
#ifndef RND_PRODQUOT
#ifndef Honor_FLT_ROUNDS
		&& Flt_Rounds == 1
#endif
#endif
			) {
		if (!e)
			goto ret;
#ifndef ROUND_BIASED_without_Round_Up
		if (e > 0) {
			if (e <= Ten_pmax) {
#ifdef VAX
				goto vax_ovfl_check;
#else
#ifdef Honor_FLT_ROUNDS
    /*正确轮数flt_轮数=2或3*/
				if (sign) {
					rv.d = -rv.d;
					sign = 0;
					}
#endif
    /*rv=*/四舍五入乘积（dVal（&rv），tens[e]）；
    Goto RET；
第二节
    }
   i=dbl_dig-nd；
   如果（e<=tenpmax+i）
    /*更高级的测试有时会让我们这样做
     *这适用于较大的i值。
     **/

#ifdef Honor_FLT_ROUNDS
    /*正确轮数flt_轮数=2或3*/
				if (sign) {
					rv.d = -rv.d;
					sign = 0;
					}
#endif
				e -= i;
				dval(&rv) *= tens[i];
#ifdef VAX
    /*Vax指数范围太窄了，我们必须
     *担心这里溢出…
     **/

 vax_ovfl_check:
				word0(&rv) -= P*Exp_msk1;
    /*rv=*/四舍五入乘积（dVal（&rv），tens[e]）；
    if（（word0（&rv）&exp_掩码）
     >exp_msk1*（dbl_max_exp+bias-1-p）
     Goto OVFL；
    word0（&rv）+=p*exp_msk1；

    /*RV=*/ rounded_product(dval(&rv), tens[e]);

#endif
				goto ret;
				}
			}
#ifndef Inaccurate_Divide
		else if (e >= -Ten_pmax) {
#ifdef Honor_FLT_ROUNDS
   /*正确轮数flt_轮数=2或3*/
			if (sign) {
				rv.d = -rv.d;
				sign = 0;
				}
#endif
   /*rv=*/四舍五入的_商（dval（&rv），tens[-e]）；
   Goto RET；
   }
第二节
endif/*舍入\偏向\不舍入*/

		}
	e1 += nd - k;

#ifdef IEEE_Arith
#ifdef SET_INEXACT
	bc.inexact = 1;
	if (k <= DBL_DIG)
		oldinexact = get_inexact();
#endif
#ifdef Avoid_Underflow
	bc.scale = 0;
#endif
#ifdef Honor_FLT_ROUNDS
	if (bc.rounding >= 2) {
		if (sign)
			bc.rounding = bc.rounding == 2 ? 0 : 2;
		else
			if (bc.rounding != 2)
				bc.rounding = 0;
		}
#endif
/*dif/*ieee算术

 /*获取起始近似值=rv*10**e1*/


	if (e1 > 0) {
		if ((i = e1 & 15))
			dval(&rv) *= tens[i];
		if (e1 &= ~15) {
			if (e1 > DBL_MAX_10_EXP) {
 ovfl:
    /*不能相信巨大的价值*/
#ifdef IEEE_Arith
#ifdef Honor_FLT_ROUNDS
				switch(bc.rounding) {
      /*E 0:/*朝向0*/
      案例3：/*朝向-无穷大*/

					word0(&rv) = Big0;
					word1(&rv) = Big1;
					break;
				  default:
					word0(&rv) = Exp_mask;
					word1(&rv) = 0;
				  }
/*SE/*荣誉出局*/
    word0（&rv）=exp_mask；
    Word1（& RV）＝0；
endif/*荣誉\u flt_回合*/

#ifdef SET_INEXACT
    /*设置溢出位*/
				dval(&rv0) = 1e300;
				dval(&rv0) *= dval(&rv0);
#endif
/*SE/*IEEE算术*/
    word0（&rv）=big0；
    word1（&rv）=big1；
endif/*ieee_arit*/

 range_err:
				if (bd0) {
					Bfree(bb);
					Bfree(bd);
					Bfree(bs);
					Bfree(bd0);
					Bfree(delta);
					}
#ifndef NO_ERRNO
				errno = ERANGE;
#endif
				goto ret;
				}
			e1 >>= 4;
			for(j = 0; e1 > 1; j++, e1 >>= 1)
				if (e1 & 1)
					dval(&rv) *= bigtens[j];
  /*最后一次乘法可能溢出。*/
			word0(&rv) -= P*Exp_msk1;
			dval(&rv) *= bigtens[j];
			if ((z = word0(&rv) & Exp_mask)
			 > Exp_msk1*(DBL_MAX_EXP+Bias-P))
				goto ovfl;
			if (z > Exp_msk1*(DBL_MAX_EXP+Bias-1-P)) {
    /*设置为最大数字*/
    /*（无法信任dbl_max）*/
				word0(&rv) = Big0;
				word1(&rv) = Big1;
				}
			else
				word0(&rv) += P*Exp_msk1;
			}
		}
	else if (e1 < 0) {
		e1 = -e1;
		if ((i = e1 & 15))
			dval(&rv) /= tens[i];
		if (e1 >>= 4) {
			if (e1 >= 1 << n_bigtens)
				goto undfl;
#ifdef Avoid_Underflow
			if (e1 & Scale_Bit)
				bc.scale = 2*P;
			for(j = 0; e1 > 0; j++, e1 >>= 1)
				if (e1 & 1)
					dval(&rv) *= tinytens[j];
			if (bc.scale && (j = 2*P + 1 - ((word0(&rv) & Exp_mask)
						>> Exp_shift)) > 0) {
    /*标度RV不正常；清除J低位*/
				if (j >= 32) {
					if (j > 54)
						goto undfl;
					word1(&rv) = 0;
					if (j >= 53)
					 word0(&rv) = (P+2)*Exp_msk1;
					else
					 word0(&rv) &= 0xffffffff << (j-32);
					}
				else
					word1(&rv) &= 0xffffffff << j;
				}
#else
			for(j = 0; e1 > 1; j++, e1 >>= 1)
				if (e1 & 1)
					dval(&rv) *= tinytens[j];
   /*最后一次乘法可能下溢。*/
			dval(&rv0) = dval(&rv);
			dval(&rv) *= tinytens[j];
			if (!dval(&rv)) {
				dval(&rv) = 2.*dval(&rv0);
				dval(&rv) *= tinytens[j];
#endif
				if (!dval(&rv)) {
 undfl:
					dval(&rv) = 0.;
					goto range_err;
					}
#ifndef Avoid_Underflow
				word0(&rv) = Tiny0;
				word1(&rv) = Tiny1;
    /*下面的改进将被清除
     *此近似值向上。
     **/

				}
#endif
			}
		}

 /*现在最难的部分——调整RV到正确的值*/

 /*将数字放入bd：真值=bd*10^e*/

	bc.nd = nd - nz1;
#ifndef NO_STRTOD_BIGCOMP
 /*nd0=nd0；/*仅在nd>strtod_diglim时需要，但在此处完成*/
   /*消除有关bc.nd0的错误警告*/

   /*可能未初始化。*/
	if (nd > strtod_diglim) {
  /*断言（strtod_diglim>=18）；18==比*/
  /*区分双精度值的最小小数位数*/
  /*在IEEE算法中。*/
		i = j = 18;
		if (i > nd0)
			j += bc.dplen;
		for(;;) {
			if (--j < bc.dp1 && j >= bc.dp0)
				j = bc.dp0 - 1;
			if (s0[j] != '0')
				break;
			--i;
			}
		e += nd - i;
		nd = i;
		if (nd0 > nd)
			nd0 = nd;
  /*（nd<9）/*必须重新计算y*/
   Y＝0；
   对于（i=0；i<nd0；+i）
    Y=10*Y+S0[I]--0'；
   对于（j=bc.dp1；i<nd；+i）
    Y=10*Y+S0[J++]--0'；
   }
  }
第二节
 bd0=s2b（s0，nd0，nd，y，bc.dplen）；

 为（；；）{
  bd=balloc（bd0->k）；
  Bcopy（BD，BD0）；
  bb=d2b（&rv，&bbe，&bbbits）；/*rv=bb*2^bbe*/

		bs = i2b(1);

		if (e >= 0) {
			bb2 = bb5 = 0;
			bd2 = bd5 = e;
			}
		else {
			bb2 = bb5 = -e;
			bd2 = bd5 = 0;
			}
		if (bbe >= 0)
			bb2 += bbe;
		else
			bd2 -= bbe;
		bs2 = bb2;
#ifdef Honor_FLT_ROUNDS
		if (bc.rounding != 1)
			bs2++;
#endif
#ifdef Avoid_Underflow
		Lsb = LSB;
		Lsb1 = 0;
		j = bbe - bc.scale;
  /*j+bbbits-1；/*logb（rv）*/
  j=p+1-bb位；
  如果（i<emin）/*不正常*/

			i = Emin - i;
			j -= i;
			if (i < 32)
				Lsb <<= i;
			else if (i < 52)
				Lsb1 = Lsb << (i-32);
			else
				Lsb1 = Exp_mask;
			}
/*SE/*避免下溢*/
ifdef突然下溢
IBM公司
  j=1+4*p-3-bbbits+（（bbe+bbbits-1）&3）；
否则
  j=p+1-bb位；
第二节
否则/*突然下溢*/

		j = bbe;
  /*j+bbbits-1；/*logb（rv）*/
  如果（i<emin）/*不正常*/

			j += P - Emin;
		else
			j = P + 1 - bbbits;
/*DIF/*突然下溢*/
endif/*避免下溢*/

		bb2 += j;
		bd2 += j;
#ifdef Avoid_Underflow
		bd2 += bc.scale;
#endif
		i = bb2 < bd2 ? bb2 : bd2;
		if (i > bs2)
			i = bs2;
		if (i > 0) {
			bb2 -= i;
			bd2 -= i;
			bs2 -= i;
			}
		if (bb5 > 0) {
			bs = pow5mult(bs, bb5);
			bb1 = mult(bs, bb);
			Bfree(bb);
			bb = bb1;
			}
		if (bb2 > 0)
			bb = lshift(bb, bb2);
		if (bd5 > 0)
			bd = pow5mult(bd, bd5);
		if (bd2 > 0)
			bd = lshift(bd, bd2);
		if (bs2 > 0)
			bs = lshift(bs, bs2);
		delta = diff(bb, bd);
		bc.dsign = delta->sign;
		delta->sign = 0;
		i = cmp(delta, bs);
/*ndef否\u strtod \u bigcomp/**/
  如果（bc.nd>nd&&i<=0）
   如果（bc.dSube）{
    /*必须使用bigcomp（）。*/

				req_bigcomp = 1;
				break;
				}
#ifdef Honor_FLT_ROUNDS
			if (bc.rounding != 1) {
				if (i < 0) {
					req_bigcomp = 1;
					break;
					}
				}
			else
#endif
    /*-1；/*丢弃的数字使delta变小。*/
   }
NeNFEF/**/

/*定义荣誉回合
  如果（公元前四舍五入！= 1）{
   如果（i＜0）{
    /*错误小于ULP*/

				if (!delta->x[0] && delta->wds <= 1) {
     /*准确的*/
#ifdef SET_INEXACT
					bc.inexact = 0;
#endif
					break;
					}
				if (bc.rounding) {
					if (bc.dsign) {
						adj.d = 1.;
						goto apply_adj;
						}
					}
				else if (!bc.dsign) {
					adj.d = -1.;
					if (!word1(&rv)
					 && !(word0(&rv) & Frac_mask)) {
						y = word0(&rv) & Exp_mask;
#ifdef Avoid_Underflow
						if (!bc.scale || y > 2*P*Exp_msk1)
#else
						if (y)
#endif
						  {
						  delta = lshift(delta,Log2P);
						  if (cmp(delta, bs) <= 0)
							adj.d = -0.5;
						  }
						}
 apply_adj:
/*def避免下溢/**/
     if（bc.scale&&（y=word0（&rv）&exp_mask）
      <=2*p*expmsk1）
       单词0（&adj）+=（2*p+1）*expmsk1-y；
否则
ifdef突然下溢
     if（（word0（&rv）&exp_mask）<=
       p*ExpUxMsk1）{
      word0（&rv）+=p*exp_msk1；
      dval（&rv）+=adj.d*ulp（dval（&rv））；
      word0（&rv）-=p*exp_msk1；
      }
     其他的
endif/*突然下溢*/

/*DIF/*避免下溢*/
     dval（&rv）+=adj.d*ulp（&rv）；
     }
    断裂；
    }
   adj.d=比率（delta，bs）；
   如果（adj. d＜1）
    d＝1；
   如果（adj.d<=0x7ffffffe）
    /*adj=舍入？CEIL（ADJ）：楼层（ADJ）；*/

				y = adj.d;
				if (y != adj.d) {
					if (!((bc.rounding>>1) ^ bc.dsign))
						y++;
					adj.d = y;
					}
				}
/*def避免下溢/**/
   if（bc.scale&&（y=word0（&rv）&exp_mask）<=2*p*exp_msk1）
    单词0（&adj）+=（2*p+1）*expmsk1-y；
否则
ifdef突然下溢
   if（（word0（&rv）&exp_mask）<=p*exp_msk1）
    word0（&rv）+=p*exp_msk1；
    adj.d*=ulp（dval（&rv））；
    如果（BC。d签）
     dval（&rv）+=adj.d；
    其他的
     dval（&rv）-=adj.d；
    word0（&rv）-=p*exp_msk1；
    Goto CONT；
    }
endif/*突然下溢*/

/*DIF/*避免下溢*/
   adj.d*=ulp（&rv）；
   如果（bc.dSube）{
    if（word0（&rv）==big0和word1（&rv）==big1）
     Goto OVFL；
    dval（&rv）+=adj.d；
    }
   其他的
    dval（&rv）-=adj.d；
   Goto CONT；
   }
endif/*荣誉诳flt_回合*/


		if (i < 0) {
   /*错误小于ulp的一半--检查
    *尾数的特殊情况是2的幂。
    **/

			if (bc.dsign || word1(&rv) || word0(&rv) & Bndry_mask
/*定义IEEE/
ifdef避免_下溢
    （word0（&rv）&exp_mask）<=（2*p+1）*exp_msk1
否则
    （word0（&rv）&exp_mask）<=exp_msk1
第二节
NeNFEF/**/

				) {
#ifdef SET_INEXACT
				if (!delta->x[0] && delta->wds <= 1)
					bc.inexact = 0;
#endif
				break;
				}
			if (!delta->x[0] && delta->wds <= 1) {
    /*精确结果*/
#ifdef SET_INEXACT
				bc.inexact = 0;
#endif
				break;
				}
			delta = lshift(delta,Log2P);
			if (cmp(delta, bs) > 0)
				goto drop_down;
			break;
			}
		if (i == 0) {
   /*正好在中间*/
			if (bc.dsign) {
				if ((word0(&rv) & Bndry_mask1) == Bndry_mask1
				 &&  word1(&rv) == (
#ifdef Avoid_Underflow
			(bc.scale && (y = word0(&rv) & Exp_mask) <= 2*P*Exp_msk1)
		? (0xffffffff & (0xffffffff << (2*P+1-(y>>Exp_shift)))) :
#endif
						   0xffffffff)) {
     /*基本情况——增量exponen*/
					if (word0(&rv) == Big0 && word1(&rv) == Big1)
						goto ovfl;
					word0(&rv) = (word0(&rv) & Exp_mask)
						+ Exp_msk1
#ifdef IBM
						| Exp_msk1 >> 4
#endif
						;
					word1(&rv) = 0;
#ifdef Avoid_Underflow
					bc.dsign = 0;
#endif
					break;
					}
				}
			else if (!(word0(&rv) & Bndry_mask) && !word1(&rv)) {
 drop_down:
    /*边界条件——减量指数*/
/*def突然下溢
    L=word0（&rv）&exp_mask；
IBM公司
    如果（l<expmsk1）
否则
ifdef避免_下溢
    如果（L<=（BC.比例？（2*P+1）*exp_msk1：exp_msk1）
否则
    如果（L<=expmsk1）
endif/*避免下溢*/

/*DIF/*IBM */
     {
     如果（bc.nd>nd）
      BC.UFLCHK＝1；
      断裂；
      }
     哥顿民主共和国；
     }
    L＝ExpXMGSK1；
否则/*突然下溢*/

#ifdef Avoid_Underflow
				if (bc.scale) {
					L = word0(&rv) & Exp_mask;
					if (L <= (2*P+1)*Exp_msk1) {
						if (L > (P+2)*Exp_msk1)
       /*圆偶数＝>*/
       /*接受右心室*/
							break;
      /*RV=最小非正规*/
						if (bc.nd >nd) {
							bc.uflchk = 1;
							break;
							}
						goto undfl;
						}
					}
/*DIF/*避免下溢*/
    L=（word0（&rv）&exp_mask）-exp_msk1；
endif/*突然下溢*/

				word0(&rv) = L | Bndry_mask1;
				word1(&rv) = 0xffffffff;
#ifdef IBM
				goto cont;
#else
#ifndef NO_STRTOD_BIGCOMP
				if (bc.nd > nd)
					goto cont;
#endif
				break;
#endif
				}
#ifndef ROUND_BIASED
#ifdef Avoid_Underflow
			if (Lsb1) {
				if (!(word0(&rv) & Lsb1))
					break;
				}
			else if (!(word1(&rv) & Lsb))
				break;
#else
			if (!(word1(&rv) & LSB))
				break;
#endif
#endif
			if (bc.dsign)
#ifdef Avoid_Underflow
				dval(&rv) += sulp(&rv, &bc);
#else
				dval(&rv) += ulp(&rv);
#endif
#ifndef ROUND_BIASED
			else {
#ifdef Avoid_Underflow
				dval(&rv) -= sulp(&rv, &bc);
#else
				dval(&rv) -= ulp(&rv);
#endif
#ifndef Sudden_Underflow
				if (!dval(&rv)) {
					if (bc.nd >nd) {
						bc.uflchk = 1;
						break;
						}
					goto undfl;
					}
#endif
				}
#ifdef Avoid_Underflow
			bc.dsign = 1 - bc.dsign;
#endif
#endif
			break;
			}
		if ((aadj = ratio(delta, bs)) <= 2.) {
			if (bc.dsign)
				aadj = aadj1 = 1.;
			else if (word1(&rv) || word0(&rv) & Bndry_mask) {
#ifndef Sudden_Underflow
				if (word1(&rv) == Tiny1 && !word0(&rv)) {
					if (bc.nd >nd) {
						bc.uflchk = 1;
						break;
						}
					goto undfl;
					}
#endif
				aadj = 1.;
				aadj1 = -1.;
				}
			else {
    /*特例——FLT基数的幂*/
    /*四舍五入…*/

				if (aadj < 2./FLT_RADIX)
					aadj = 1./FLT_RADIX;
				else
					aadj *= 0.5;
				aadj1 = -aadj;
				}
			}
		else {
			aadj *= 0.5;
			aadj1 = bc.dsign ? aadj : -aadj;
#ifdef Check_FLT_ROUNDS
			switch(bc.rounding) {
    /*E 2:/*朝向+无穷大*/
     AADJ1-＝0.5；
     断裂；
    案例0:/*朝向0*/

    /*E 3:/*朝向-无穷大*/
     AADJ1+＝0.5；
    }
否则
   如果（flt_rounds==0）
    AADJ1+＝0.5；
endif/*检查\flt_轮*/

			}
		y = word0(&rv) & Exp_mask;

  /*检查是否溢出*/

		if (y == Exp_msk1*(DBL_MAX_EXP+Bias-1)) {
			dval(&rv0) = dval(&rv);
			word0(&rv) -= P*Exp_msk1;
			adj.d = aadj1 * ulp(&rv);
			dval(&rv) += adj.d;
			if ((word0(&rv) & Exp_mask) >=
					Exp_msk1*(DBL_MAX_EXP+Bias-P)) {
				if (word0(&rv0) == Big0 && word1(&rv0) == Big1)
					goto ovfl;
				word0(&rv) = Big0;
				word1(&rv) = Big1;
				goto cont;
				}
			else
				word0(&rv) += P*Exp_msk1;
			}
		else {
#ifdef Avoid_Underflow
			if (bc.scale && y <= 2*P*Exp_msk1) {
				if (aadj <= 0x7fffffff) {
					if ((z = aadj) <= 0)
						z = 1;
					aadj = z;
					aadj1 = bc.dsign ? aadj : -aadj;
					}
				dval(&aadj2) = aadj1;
				word0(&aadj2) += (2*P+1)*Exp_msk1 - y;
				aadj1 = dval(&aadj2);
				adj.d = aadj1 * ulp(&rv);
				dval(&rv) += adj.d;
				if (rv.d == 0.)
#ifdef NO_STRTOD_BIGCOMP
					goto undfl;
#else
					{
					req_bigcomp = 1;
					break;
					}
#endif
				}
			else {
				adj.d = aadj1 * ulp(&rv);
				dval(&rv) += adj.d;
				}
#else
#ifdef Sudden_Underflow
			if ((word0(&rv) & Exp_mask) <= P*Exp_msk1) {
				dval(&rv0) = dval(&rv);
				word0(&rv) += P*Exp_msk1;
				adj.d = aadj1 * ulp(&rv);
				dval(&rv) += adj.d;
#ifdef IBM
				if ((word0(&rv) & Exp_mask) <  P*Exp_msk1)
#else
				if ((word0(&rv) & Exp_mask) <= P*Exp_msk1)
#endif
					{
					if (word0(&rv0) == Tiny0
					 && word1(&rv0) == Tiny1) {
						if (bc.nd >nd) {
							bc.uflchk = 1;
							break;
							}
						goto undfl;
						}
					word0(&rv) = Tiny0;
					word1(&rv) = Tiny1;
					goto cont;
					}
				else
					word0(&rv) -= P*Exp_msk1;
				}
			else {
				adj.d = aadj1 * ulp(&rv);
				dval(&rv) += adj.d;
				}
/*SE/*突然下溢*/
   /*计算ADJ，以便IEEE舍入规则
    *在某些半程情况下，正确地绕过RV+ADJ。
    *如果RV*ULP（RV）不正常（即，
    *Y<=（P-1）*expmsk1），我们必须调整aadj以避免
    *位丢失到非规范化的故障；
    *示例：1.2E-307。
    **/

			if (y <= (P-1)*Exp_msk1 && aadj > 1.) {
				aadj1 = (double)(int)(aadj + 0.5);
				if (!bc.dsign)
					aadj1 = -aadj1;
				}
			adj.d = aadj1 * ulp(&rv);
			dval(&rv) += adj.d;
/*DIF/*突然下溢*/
endif/*避免下溢*/

			}
		z = word0(&rv) & Exp_mask;
#ifndef SET_INEXACT
		if (bc.nd == nd) {
#ifdef Avoid_Underflow
		if (!bc.scale)
#endif
		if (y == z) {
   /*我们现在能停下来吗？*/
			L = (Long)aadj;
			aadj -= L;
   /*下面的公差是保守的。*/
			if (bc.dsign || word1(&rv) || word0(&rv) & Bndry_mask) {
				if (aadj < .4999999 || aadj > .5000001)
					break;
				}
			else if (aadj < .4999999/FLT_RADIX)
				break;
			}
		}
#endif
 cont:
		Bfree(bb);
		Bfree(bd);
		Bfree(bs);
		Bfree(delta);
		}
	Bfree(bb);
	Bfree(bd);
	Bfree(bs);
	Bfree(bd0);
	Bfree(delta);
#ifndef NO_STRTOD_BIGCOMP
	if (req_bigcomp) {
		bd0 = 0;
		bc.e0 += nz1;
		bigcomp(&rv, s0, &bc);
		y = word0(&rv) & Exp_mask;
		if (y == Exp_mask)
			goto ovfl;
		if (y == 0 && rv.d == 0.)
			goto undfl;
		}
#endif
#ifdef SET_INEXACT
	if (bc.inexact) {
		if (!oldinexact) {
			word0(&rv0) = Exp_1 + (70 << Exp_shift);
			word1(&rv0) = 0;
			dval(&rv0) += 1.;
			}
		}
	else if (!oldinexact)
		clear_inexact();
#endif
#ifdef Avoid_Underflow
	if (bc.scale) {
		word0(&rv0) = Exp_1 - 2*P*Exp_msk1;
		word1(&rv0) = 0;
		dval(&rv) *= dval(&rv0);
#ifndef NO_ERRNO
  /*尽量避免测试8087寄存器值的错误*/
#ifdef IEEE_Arith
		if (!(word0(&rv) & Exp_mask))
#else
		if (word0(&rv) == 0 && word1(&rv) == 0)
#endif
			errno = ERANGE;
#endif
		}
/*DIF/*避免下溢*/
ifdef设置不精确
 如果（bc.不精确&！（word0（&rv）&exp_mask））
  /*设置下溢位*/

		dval(&rv0) = 1e-300;
		dval(&rv0) *= dval(&rv0);
		}
#endif
 ret:
	if (se)
		*se = (char *)s;
	return sign ? -dval(&rv) : dval(&rv);
	}

#ifndef MULTIPLE_THREADS
 static char *dtoa_result;
#endif

 static char *
#ifdef KR_headers
rv_alloc(i) int i;
#else
rv_alloc(int i)
#endif
{
	int j, k, *r;

	j = sizeof(ULong);
	for(k = 0;
		sizeof(Bigint) - sizeof(ULong) - sizeof(int) + j <= i;
		j <<= 1)
			k++;
	r = (int*)Balloc(k);
	*r = k;
	return
#ifndef MULTIPLE_THREADS
	dtoa_result =
#endif
		(char *)(r+1);
	}

 static char *
#ifdef KR_headers
nrv_alloc(s, rve, n) char *s, **rve; int n;
#else
nrv_alloc(const char *s, char **rve, int n)
#endif
{
	char *rv, *t;

	t = rv = rv_alloc(n);
	while((*t = *s++)) t++;
	if (rve)
		*rve = t;
	return rv;
	}

/*free dtoa（s）必须用于释放由dtoa返回的值
 *定义多个线程时。它应该在所有情况下使用，
 *但为了与早期版本的DTOA保持一致，它是可选的。
 *未定义多个线程时。
 **/


 void
#ifdef KR_headers
freedtoa(s) char *s;
#else
freedtoa(char *s)
#endif
{
	Bigint *b = (Bigint *)((int *)s - 1);
	b->maxwds = 1 << (b->k = *(int*)b);
	Bfree(b);
#ifndef MULTIPLE_THREADS
	if (s == dtoa_result)
		dtoa_result = 0;
#endif
	}

/*IEEE算术（DMG）的DTOA：将double转换为ASCII字符串。
 *
 
 *Guy L.Steele，Jr.和Jon L.White[proc.ACM Sigplan'90，第112-126页]。
 *
 *修改：
 * 1。我们使用简单的数字高估而不是迭代
 *确定k=楼层（log10（d））。我们的规模相关
 *使用O（log2（k））而不是O（k）乘法的数量。
 * 2。对于某些模式>2（对应于ECVT和FCVT），我们不
 *尝试严格从左到右生成数字。相反，我们
 *使用更少的位进行计算，必要时传播进位
 *当最后一个数字四舍五入时。这通常更快。
 * 3。假设输入最接近四舍五入，
 *模式0将1E23呈现为1E23，而不是9.999999999E22。
 *也就是说，当
 *圆最近规则将给出相同的浮点值。
 *对于停止试验的满意程度
 *不平等。
 * 4。我们从相关的
 *数量。
 * 5。当转换小于1E16的浮点整数时，
 *我们使用浮点运算而不是诉诸
 *到多个精度整数。
 * 6。当要求生成少于15位数字时，我们首先尝试
 *为了通过浮点运算，我们求助于
 *只有当我们不能使用多精度整数算法时
 *保证浮点计算已经给出
 *正确的四舍五入结果。对于K请求的数字和
 *“均匀”分布输入，概率为
 *类似于10^（k-15），我们必须求助于
 *计算。
 **/


 char *
dtoa
#ifdef KR_headers
	(dd, mode, ndigits, decpt, sign, rve)
	double dd; int mode, ndigits, *decpt, *sign; char **rve;
#else
	(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve)
#endif
{
 /*参数ndigits、decpt、sign与这些类似
 在ECVT和FCVT中；从
 返回的字符串。如果不为空，*rve设置为点
 返回值的结尾。如果d是+无穷大或NaN，
 然后*decpt设置为9999。

 模式：
  0==>读取时产生d的最短字符串
   四舍五入到最近。
  1==>类似于0，但有钢白色停止规则；
   例如，使用IEEE P754算法，模式0给出
   1E23，而模式1给出9.99999999999E22。
  2==>最大（1，ndigits）有效数字。这给出了一个
   返回值与ECVT相似，但
   尾随的零被抑制。
  3=>通过ndigits超过小数点。这个
   给出一个类似于fcvt的返回值，
   除了尾随的零被抑制，以及
   ndigits可以是负数。
  4,5=>分别与2和3相似，但（in
   四舍五入最近模式），模式0至
   可能返回一个较短的字符串，该字符串舍入为d。
   与IEEE算术和编译
   -dhonor_flt_轮，模式4和5的行为相同
   作为模式2和3，当FLTU回合！= 1。
  6-9==>与模式-4类似的调试模式：不要尝试
   快速浮点估计（如果适用）。

  除0-9以外的模式值被视为模式0。

  为返回值分配了足够的空间
  保留被抑制的尾随零。
 **/


	int bbits, b2, b5, be, dig, i, ieps, ilim, ilim0, ilim1,
		j, j1, k, k0, k_check, leftright, m2, m5, s2, s5,
		spec_case, try_quick;
	Long L;
#ifndef Sudden_Underflow
	int denorm;
	ULong x;
#endif
	Bigint *b, *b1, *delta, *mlo, *mhi, *S;
	U d2, eps, u;
	double ds;
	char *s, *s0;
#ifndef No_leftright
#ifdef IEEE_Arith
	U eps1;
#endif
#endif
#ifdef SET_INEXACT
	int inexact, oldinexact;
#endif
/*定义荣誉回合
 整数舍入；
ifdef trust_flt_rounds/*仅当flt_rounds确实有效时才定义此项！*/

	Rounding = Flt_Rounds;
/*SE/*}{*/
 舍入＝1；
 开关（fegetRound（））
   案例fe_towardzero：舍入=0；中断；
   案例fe_向上：四舍五入=2；中断；
   案例fe_向下：四舍五入=3；
   }
In·NeNF/*}*/

/*DIF/*}*

ifndef多线程
 如果（DTOA_结果）
  freedtoa（dtoaou结果）；
  DTOA_结果=0；
  }
第二节

 UD＝DD；
 if（word0（&u）&sign_
  /*为所有内容设置标志，包括0和nan*/

		*sign = 1;
  /*d0（&u）&=~符号位；/*清除符号位*/
  }
 其他的
  ＊符号＝0；

如果定义（IEEE算术）+定义（VAX）
ifdef-ieee-arith
 if（（word0（&u）&exp_mask）==exp_mask）
否则
 如果（word0（&u）==0x8000）
第二节
  {
  /*无穷大或NaN*/

		*decpt = 9999;
#ifdef IEEE_Arith
		if (!word1(&u) && !(word0(&u) & 0xfffff))
			return nrv_alloc("Infinity", rve, 8);
#endif
		return nrv_alloc("NaN", rve, 3);
		}
#endif
#ifdef IBM
 /*L（&U）+=0；/*正常化*/
第二节
 如果（！）DVAL（& u）{
  ＊DEPT＝1；
  返回nrv_alloc（“0”，rve，1）；
  }

ifdef设置不精确
 try_quick=oldincact=get_incact（）；
 不精确＝1；
第二节
IFDEF荣誉\U FLTU回合
 如果（四舍五入>=2）
  如果（*符号）
   舍入=舍入=2？0：2；
  其他的
   如果（四舍五入）！= 2）
    舍入＝0；
  }
第二节

 b=d2b（&u，&be，&bbits）；
ifdef突然下溢
 i=（int）（word0（&u）>>exp_shift1&（exp_mask>>exp_shift1））；
否则
 if（（i=（int）（word0（&u）>>exp诳shift1&（exp诳mask>>exp诳shift1）））；
第二节
  dVal（&d2）=dVal（&u）；
  单词0（&d2）&=frac_mask1；
  word0（&d2）=exp_11；
IBM公司
  如果（j=11-hi0bits（word0（&d2）&frac_mask）
   dVal（&d2）/=1<<j；
第二节

  /*对数（x）~=~对数（1.5）+（x-1.5）/1.5
   *log10（x）=log（x）/log（10）
   *~=~对数（1.5）/对数（10）+（x-1.5）/（1.5*对数（10））
   *log10（d）=（i-偏差）*log 2/log10+log10（d2）
   *
   *这建议通过以下方法计算k到log10（d）的近似值：
   *
   *K=（I-偏差）*0.301029995663981
   （（d2-1.5）*0.289529654602168+0.176091259055681）；
   *
   *我们希望K太大而不是太小。
   *一阶泰勒级数近似的误差
   *对我们有利，所以我们只要把常数凑齐就行了。
   *补偿乘法中的任何错误
   （i-偏差）0.301029995663981；因为i-偏差<=1077，
   *和1077*0.30103*2^-52~=~7.2e-14，
   *常数项加上1e-13就足够了。
   *因此，我们将常数项调整为0.1760912590558。
   （我们可以通过调用log10得到更精确的k，
   *但这可能不值得。）
   **/


		i -= Bias;
#ifdef IBM
		i <<= 2;
		i += j;
#endif
#ifndef Sudden_Underflow
		denorm = 0;
		}
	else {
  /*d不规范化*/

		i = bbits + be + (Bias + (P-1) - 1);
		x = i > 32  ? word0(&u) << (64 - i) | word1(&u) >> (i - 32)
			    : word1(&u) << (32 - i);
		dval(&d2) = x;
  /*d0（&d2）-=31*expmsk1；/*调整指数*/
  i-=（偏压+（P-1）-1）+1；
  DENORM＝1；
  }
第二节
 ds=（dval（&d2）-1.5）*0.289529654602168+0.1760912590558+i*0.301029995663981；
 k=（int）DS；
 （DS＜0）。＆DS！= K）
  k--；/*想要k=楼层（ds）*/

	k_check = 1;
	if (k >= 0 && k <= Ten_pmax) {
		if (dval(&u) < tens[k])
			k--;
		k_check = 0;
		}
	j = bbits - i - 1;
	if (j >= 0) {
		b2 = 0;
		s2 = j;
		}
	else {
		b2 = -j;
		s2 = 0;
		}
	if (k >= 0) {
		b5 = 0;
		s5 = k;
		s2 += k;
		}
	else {
		b2 -= k;
		b5 = -k;
		s5 = 0;
		}
	if (mode < 0 || mode > 9)
		mode = 0;

#ifndef SET_INEXACT
#ifdef Check_FLT_ROUNDS
	try_quick = Rounding == 1;
#else
	try_quick = 1;
#endif
/*DIF/*设置不精确*/

 如果（模式＞5）{
  模式＝4；
  TyyQuy＝0；
  }
 左旋＝1；
 ilim=ilim1=-1；/*案例0和1的值；在此完成*/

    /*沉默错误的“GCC-墙”警告。*/
	switch(mode) {
		case 0:
		case 1:
			i = 18;
			ndigits = 0;
			break;
		case 2:
			leftright = 0;
   /*不中断*/
		case 4:
			if (ndigits <= 0)
				ndigits = 1;
			ilim = ilim1 = i = ndigits;
			break;
		case 3:
			leftright = 0;
   /*不中断*/
		case 5:
			i = ndigits + k + 1;
			ilim = i;
			ilim1 = i - 1;
			if (i <= 0)
				i = 1;
		}
	s = s0 = rv_alloc(i);

#ifdef Honor_FLT_ROUNDS
	if (mode > 1 && Rounding != 1)
		leftright = 0;
#endif

	if (ilim >= 0 && ilim <= Quick_max && try_quick) {

  /*试着通过浮点运算。*/

		i = 0;
		dval(&d2) = dval(&u);
		k0 = k;
		ilim0 = ilim;
  /*S=2；/*保守*/
  如果（k＞0）{
   ds=十[k&0xf]；
   J＝K＞4；
   如果（J&Bletch）
    /*防止溢出*/

				j &= Bletch - 1;
				dval(&u) /= bigtens[n_bigtens-1];
				ieps++;
				}
			for(; j; j >>= 1, i++)
				if (j & 1) {
					ieps++;
					ds *= bigtens[i];
					}
			dval(&u) /= ds;
			}
		else if ((j1 = -k)) {
			dval(&u) *= tens[j1 & 0xf];
			for(j = j1 >> 4; j; j >>= 1, i++)
				if (j & 1) {
					ieps++;
					dval(&u) *= bigtens[i];
					}
			}
		if (k_check && dval(&u) < 1. && ilim > 0) {
			if (ilim1 <= 0)
				goto fast_failed;
			ilim = ilim1;
			k--;
			dval(&u) *= 10.;
			ieps++;
			}
		dval(&eps) = ieps*dval(&u) + 7.;
		word0(&eps) -= (P-1)*Exp_msk1;
		if (ilim == 0) {
			S = mhi = 0;
			dval(&u) -= 5.;
			if (dval(&u) > dval(&eps))
				goto one_digit;
			if (dval(&u) < -dval(&eps))
				goto no_digits;
			goto fast_failed;
			}
#ifndef No_leftright
		if (leftright) {
   /*仅使用Steele&White方法
    *需要生成数字。
    **/

			dval(&eps) = 0.5/tens[ilim-1] - dval(&eps);
#ifdef IEEE_Arith
			if (k0 < 0 && j1 >= 307) {
    /*1.d=1.01e256；/*1.01允许在接下来的几行中进行舍入*/
    word0（&eps1）-=expmsk1*（偏压+p-1）；
    dVal（&eps1）*=Tens[j1&0xf]；
    对于（i=0，j=（j1-256）>>4；j；j>>=1，i++）
     如果（j和1）
      dval（&eps1）*=bigtens[i]；
    如果（eps.d<eps1.d）
     EPS.D＝EPS1.D；
    }
第二节
   对于（i＝0；；）{
    L＝DVAL（& U）；
    DVAL（和U）-L；
    *s++='0'+（int）l；
    如果（1）。-dval（&u）<dval（&eps））
     Goto BuMPUP；
    如果（dVal（&u）<dVal（&eps）
     Goto RIT1；
    如果（++i>=ilim）
     断裂；
    dval（&eps）*=10.；
    dval（&u）*=10.；
    }
   }
  否则{
第二节
   /*生成ILIM数字，然后修复它们。*/

			dval(&eps) *= tens[ilim-1];
			for(i = 1;; i++, dval(&u) *= 10.) {
				L = (Long)(dval(&u));
				if (!(dval(&u) -= L))
					ilim = i;
				*s++ = '0' + (int)L;
				if (i == ilim) {
					if (dval(&u) > 0.5 + dval(&eps))
						goto bump_up;
					else if (dval(&u) < 0.5 - dval(&eps)) {
						while(*--s == '0');
						s++;
						goto ret1;
						}
					break;
					}
				}
#ifndef No_leftright
			}
#endif
 fast_failed:
		s = s0;
		dval(&u) = dval(&d2);
		k = k0;
		ilim = ilim0;
		}

 /*我们有一个“小”整数吗？*/

	if (be >= 0 && k <= Int_max) {
  /*对。*/
		ds = tens[k];
		if (ndigits < 0 && ilim <= 0) {
			S = mhi = 0;
			if (ilim < 0 || dval(&u) <= 5*ds)
				goto no_digits;
			goto one_digit;
			}
		for(i = 1;; i++, dval(&u) *= 10.) {
			L = (Long)(dval(&u) / ds);
			dval(&u) -= L*ds;
#ifdef Check_FLT_ROUNDS
   /*如果flt_rounds==2，l通常高1*/
			if (dval(&u) < 0) {
				L--;
				dval(&u) += ds;
				}
#endif
			*s++ = '0' + (int)L;
			if (!dval(&u)) {
#ifdef SET_INEXACT
				inexact = 0;
#endif
				break;
				}
			if (i == ilim) {
#ifdef Honor_FLT_ROUNDS
				if (mode > 1)
				switch(Rounding) {
				  case 0: goto ret1;
				  case 2: goto bump_up;
				  }
#endif
				dval(&u) += dval(&u);
#ifdef ROUND_BIASED
				if (dval(&u) >= ds)
#else
				if (dval(&u) > ds || (dval(&u) == ds && L & 1))
#endif
					{
 bump_up:
					while(*--s == '9')
						if (s == s0) {
							k++;
							*s = '0';
							break;
							}
					++*s++;
					}
				break;
				}
			}
		goto ret1;
		}

	m2 = b2;
	m5 = b5;
	mhi = mlo = 0;
	if (leftright) {
		i =
#ifndef Sudden_Underflow
			denorm ? be + (Bias + (P-1) - 1 + 1) :
#endif
#ifdef IBM
			1 + 4*P - 3 - bbits + ((bbits + be - 1) & 3);
#else
			1 + P - bbits;
#endif
		b2 += i;
		s2 += i;
		mhi = i2b(1);
		}
	if (m2 > 0 && s2 > 0) {
		i = m2 < s2 ? m2 : s2;
		b2 -= i;
		m2 -= i;
		s2 -= i;
		}
	if (b5 > 0) {
		if (leftright) {
			if (m5 > 0) {
				mhi = pow5mult(mhi, m5);
				b1 = mult(mhi, b);
				Bfree(b);
				b = b1;
				}
			if ((j = b5 - m5))
				b = pow5mult(b, j);
			}
		else
			b = pow5mult(b, b5);
		}
	S = i2b(1);
	if (s5 > 0)
		S = pow5mult(S, s5);

 /*检查有无特殊情况，即d是2的标准化功率。*/

	spec_case = 0;
	if ((mode < 2 || leftright)
#ifdef Honor_FLT_ROUNDS
			&& Rounding == 1
#endif
				) {
		if (!word1(&u) && !(word0(&u) & Bndry_mask)
#ifndef Sudden_Underflow
		 && word0(&u) & (Exp_mask & ~Exp_msk1)
#endif
				) {
   /*特殊情况*/
			b2 += Log2P;
			s2 += Log2P;
			spec_case = 1;
			}
		}

 /*安排方便的商计算：
  *必要时向左移位，使除数有4个前导0位。
  *
  *也许我们应该只计算一次S的前28位
  *对所有人来说，通过他们，然后转移到Quorem，所以
  *可以进行移位和ORS来计算q的分子。
  **/

	i = dshift(S, s2);
	b2 += i;
	m2 += i;
	s2 += i;
	if (b2 > 0)
		b = lshift(b, b2);
	if (s2 > 0)
		S = lshift(S, s2);
	if (k_check) {
		if (cmp(b,S) < 0) {
			k--;
   /*multad（b，10，0）；/*我们搞错了k估计值*/
   如果（左）
    mhi=multad（mhi，10，0）；
   ILIM1；
   }
  }
 如果（ilim<=0&（模式==3模式==5））
  如果（ilim<0 cmp（b，s=multad（s，5,0））<=0）
   /*无数字，FCVT样式*/

 no_digits:
			k = -1 - ndigits;
			goto ret;
			}
 one_digit:
		*s++ = '1';
		k++;
		goto ret;
		}
	if (leftright) {
		if (m2 > 0)
			mhi = lshift(mhi, m2);

  /*计算MLO—检查特殊情况
   *d是2的标准化幂。
   **/


		mlo = mhi;
		if (spec_case) {
			mhi = Balloc(mhi->k);
			Bcopy(mhi, mlo);
			mhi = lshift(mhi, Log2P);
			}

		for(i = 1;;i++) {
			dig = quorem(b,S) + '0';
   /*我们还有最短的十进制字符串吗
    *这将四舍五入到D？
    **/

			j = cmp(b, mlo);
			delta = diff(S, mhi);
			j1 = delta->sign ? 1 : cmp(b, delta);
			Bfree(delta);
#ifndef ROUND_BIASED
			if (j1 == 0 && mode != 1 && !(word1(&u) & 1)
#ifdef Honor_FLT_ROUNDS
				&& Rounding >= 1
#endif
								   ) {
				if (dig == '9')
					goto round_9_up;
				if (j > 0)
					dig++;
#ifdef SET_INEXACT
				else if (!b->x[0] && b->wds <= 1)
					inexact = 0;
#endif
				*s++ = dig;
				goto ret;
				}
#endif
			if (j < 0 || (j == 0 && mode != 1
#ifndef ROUND_BIASED
							&& !(word1(&u) & 1)
#endif
					)) {
				if (!b->x[0] && b->wds <= 1) {
#ifdef SET_INEXACT
					inexact = 0;
#endif
					goto accept_dig;
					}
#ifdef Honor_FLT_ROUNDS
				if (mode > 1)
				 switch(Rounding) {
				  case 0: goto accept_dig;
				  case 2: goto keep_dig;
				  }
/*DIF/*荣誉回合*/
    如果（J1＞0）{
     b=lshift（b，1）；
     J1＝CMP（B，S）；
ifdef round覕u偏向
     如果（J1>＝0／*）*/

#else
					if ((j1 > 0 || (j1 == 0 && dig & 1))
#endif
					&& dig++ == '9')
						goto round_9_up;
					}
 accept_dig:
				*s++ = dig;
				goto ret;
				}
			if (j1 > 0) {
#ifdef Honor_FLT_ROUNDS
				if (!Rounding)
					goto accept_dig;
#endif
    /*（dig='9'）/*如果i==1*/
 圆形的：
     * S+++＝“9”；
     转到舍入；
     }
    *S++=挖掘+ 1；
    Goto RET；
    }
IFDEF荣誉\U FLTU回合
 KeePiGig：
第二节
   *S++=挖掘；
   如果（i= iLIM）
    断裂；
   b=multad（b，10，0）；
   如果（MLO＝＝MHI）
    mlo=mhi=multad（mhi，10，0）；
   否则{
    mlo=multad（mlo，10，0）；
    mhi=multad（mhi，10，0）；
    }
   }
  }
 其他的
  对于（i=1；；i++）
   *s++=dig=quorem（b，s）+‘0’；
   如果（！）b->x[0]&&b->wds<=1）
ifdef设置不精确
    不精确＝0；
第二节
    Goto RET；
    }
   如果（i>＝ILIM）
    断裂；
   b=multad（b，10，0）；
   }

 /*四舍五入最后一位*/


#ifdef Honor_FLT_ROUNDS
	switch(Rounding) {
	  case 0: goto trimzeros;
	  case 2: goto roundoff;
	  }
#endif
	b = lshift(b, 1);
	j = cmp(b, S);
#ifdef ROUND_BIASED
	if (j >= 0)
#else
	if (j > 0 || (j == 0 && dig & 1))
#endif
		{
 roundoff:
		while(*--s == '9')
			if (s == s0) {
				k++;
				*s++ = '1';
				goto ret;
				}
		++*s++;
		}
	else {
#ifdef Honor_FLT_ROUNDS
 trimzeros:
#endif
		while(*--s == '0');
		s++;
		}
 ret:
	Bfree(S);
	if (mhi) {
		if (mlo && mlo != mhi)
			Bfree(mlo);
		Bfree(mhi);
		}
 ret1:
#ifdef SET_INEXACT
	if (inexact) {
		if (!oldinexact) {
			word0(&u) = Exp_1 + (70 << Exp_shift);
			word1(&u) = 0;
			dval(&u) += 1.;
			}
		}
	else if (!oldinexact)
		clear_inexact();
#endif
	Bfree(b);
	*s = 0;
	*decpt = k + 1;
	if (rve)
		*rve = s;
	return s0;
	}
#ifdef __cplusplus
}
#endif
