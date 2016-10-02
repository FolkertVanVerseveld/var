/* public domain */
/* http://unix.stackexchange.com/questions/43539/what-do-the-flags-in-proc-cpuinfo-mean/219674#219674 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define BUFSZ 4096

static char *cpupath = "/proc/cpuinfo";
static FILE *file = NULL;
static char buf[BUFSZ];

static const struct opt {
	const char *flag;
	const char *desc;
} tbl[] = {
	/* intel */
	{"fpu"    , "onboard floating point unit"},
	{"vme"    , "virtual mode extensions"},
	{"de"     , "debugging extensions"},
	{"pse"    , "page size extensions up to 4MB"},
	{"tsc"    , "time stamp counter"},
	{"msr"    , "model-specific registers"},
	{"pae"    , "4GB+ physical address extensions"},
	{"mce"    , "machine check exception"},
	{"cx8"    , "64-bit comparing and swapping"},
	{"apic"   , "onboard advanced programmable interrupt controller"},
	{"sep"    , "seperate non-interrupt system call and return"},
	{"mtrr"   , "memory type range registers"},
	{"pge"    , "page global enable"},
	{"mca"    , "machine check architecture"},
	{"cmov"   , "conditional moving"},
	{"pat"    , "page attribute table"},
	{"pse36"  , "36-bit page size extensions"},
	{"pn"     , "processor serial number"},
	{"clflush", "cache line flushing"},
	{"dts"    , "buffered debug and profiling storage"},
	{"acpi"   , "onboard advanced configuration and power interface"},
	{"mmx"    , "multimedia extensions"},
	{"fxsr"   , "fast floating point unit save and restore"},
	{"sse"    , "streaming single instruction multiple data extensions version 1"},
	{"sse2"   , "streaming single instruction multiple data extensions version 2"},
	{"ss"     , "self snooping"},
	{"ht"     , "threading"},
	{"tm"     , "automatic clock control"},
	{"ia64"   , "itanium 64-bit instruction set architecture"},
	{"pbe"    , "pending break enable wakeup"},
	/* AMD */
	{"syscall" , "seperate non-interrupt system call and return"},
	{"mp"      , "multiprocessing core"},
	{"nx"      , "code and data segregation"},
	{"mmxext"  , "vector processing multimedia extensions"},
	{"fxsr_opt", "fast floating point unit save and restore optimizations"},
	{"pdpe1gb" , "page size extensions up to 1GB"},
	{"rdtscp"  , "time stamp counter and processor identification"},
	{"lm"      , "64 bit mode"},
	{"3dnowext", "vector processing multimedia extensions version 2"},
	{"3dnow"   , "vector processing"},
	/* transmeta */
	{"recovery", "central processing unit recovery"},
	{"longrun" , "longrun power control"},
	{"lrti"    , "longrun table interface"},
	/* linux */
	{"cxmxx"             , "cyrix specific multimedia extenions"},
	{"k6_mtrr"           , "nonstandard AMD K6 memory type range registers"},
	{"cyrix_arr"         , "nonstandard cyrix memory type range registers"},
	{"centaur_mcr"       , "nonstandard centaur memory type range registers"},
	{"constant_tsc"      , "constant time stamp counter"},
	{"up"                , "symmetric multiprocessing kernel running on up"},
	{"arch_perfmon"      , "architectural performance monitoring"},
	{"pebs"              , "precise event-based sampling"},
	{"bts"               , "branch trace store"},
	{"rep_good"          , "repeat microcode works good"},
	{"nopl"              , "nopl"},
	{"xtopology"         , "topology enumeration extensions"},
	{"tsc_reliable"      , "time stamp counter reliable"},
	{"nonstop_tsc"       , "time stamp counter does not stop in C states"},
	{"extd_apicid"       , "extended advanced programmable interrupt controller"},
	{"amd_dcm"           , "multi-node processor"},
	{"aperfmperf"        , "aperfmperf"},
	{"eagerfpu"          , "non-lazy floating point unit restore"},
	{"nonstop_tsc_s3"    , "time stamp counter does not stop in S3 state"},
	/* intel */
	{"pni"               , "streaming single instruction multiple data extensions version 3"},
	{"pclmulqdq"         , "carry-less multiplication of quadword"},
	{"dtes64"            , "64-bit debug store"},
	{"monitor"           , "prescott new instructions monitor supplement"},
	{"ds_cpl"            , "CPL qual. debug store"},
	{"vmx"               , "hardware virtualization"},
	{"smx"               , "NSA DRM lock-in"},
	{"est"               , "dynamic clock speed"},
	{"tm2"               , "power consumption control"},
	{"ssse3"             , "streaming single instruction multiple data extensions version 3.1"},
	{"cid"               , "context identification"},
	{"fma"               , "fused multiply-add"},
	{"cx16"              , "atomic concurrent programming primitive"},
	{"xtpr"              , "task priority messages multiplexing"},
	{"pdcm"              , "performance capabilities"},
	{"pcid"              , "process context identification"},
	{"dca"               , "direct cache access"},
	{"sse4_1"            , "streaming single instruction multiple data extensions version 4.1"},
	{"sse4_2"            , "streaming single instruction multiple data extensions version 4.2"},
	{"x2apic"            , "32-bit symmetric multiprocessor advanced programmable interrupt controller addressing"},
	{"movbe"             , "swap moved data"},
	{"popcnt"            , "bit counting"},
	{"tsc_deadline_timer", "deadline time stamp counter"},
	{"aes"               , "advanced encryption standard version 1"},
	{"aes-ni"            , "advanced encryption standard version 2"},
	{"xsave"             , "extended processor save state"},
	{"avx"               , "advanced vector extensions"},
	{"f16c"              , "16-bit half precision floating point conversion"},
	{"rdrand"            , "onboard intel hardware random number generator"},
	{"hypervisor"        , "virtual machine monitor"},
	/* VIA/Cyrix/Centaur */
	{"rng"               , "onboard non-intel hardware random number generator"},
	{"ace"               , "disabled onboard extended cryptography version 1"},
	{"ace_en"            , "enabled onboard extended cryptography version 1"},
	{"ace2"              , "disabled onboard extended cryptography version 2"},
	{"ace2_en"           , "enabled onboard extended cryptography version 2"},
	{"phe"               , "disabled padlock hash"},
	{"phe_en"            , "enabled padlock hash"},
	{"pmm"               , "disabled padlock montgomery multiplier"},
	{"pmm_en"            , "enabled padlock montgomery multiplier"},
	/* extended AMD */
	{"lahf_lm"           , "64-bit mode accumulator flag manipulation"},
	{"cmp_legacy"        , "invalid threading"},
	{"svm"               , "virtualization jailing"},
	{"extapic"           , "extended advanced programmable interrupt controller"},
	{"cr8_legacy"        , "32-bit cr8 legacy"},
	{"abm"               , "advanced bit manipulation"},
	{"sse4a"             , "streaming single instruction multiple data AMD extensions"},
	{"misalignsse"       , "misaligned streaming single instruction multiple data"},
	{"3dnowprefetch"     , "prefetching vector processing"},
	{"osvw"              , "operating system visible workaround active"},
	{"ibs"               , "instruction based sampling"},
	{"xop"               , "streaming single instruction multiple data extensions version 5"},
	{"skinit"            , "skinit and stgi"},
	{"wdt"               , "watchdog timer"},
	{"lwp"               , "light weight profiling"},
	/* FIXME rephrase descriptions below */
	{"fma4"              , "4 operands message authentication code instructions"},
	{"tce"               , "translation cache extension"},
	{"nodeid_msr"        , "node identifier model specific register"},
	{"tbm"               , "trailing bit manipulation"},
	{"topoext"           , "topology extensions cpuid leafs"},
	{"perfctr_core"      , "core performance counter extensions"},
	{"perfctr_nb"        , "NB performance counter extensions"},
	{"perfctr_l2"        , "level 2 cache performance counter extensions"},
	/* extended linux */
	{"ida", "Intel Dynamic Acceleration"},
	{"arat", "Always Running APIC Timer"},
	{"cpb", "AMD Core Performance Boost"},
	{"epb", "IA32_ENERGY_PERF_BIAS support"},
	{"xsaveopt", "Optimized Xsave"},
	{"pln", "Intel Power Limit Notification"},
	{"pts", "Intel Package Thermal Status"},
	{"dts", "Digital Thermal Sensor"},
	{"hw_pstate", "AMD HW-PState"},
	{"proc_feedback", "AMD ProcFeedbackInterface"},
	{"intel_pt", "Intel Processor Tracing"},
	/* virtualization */
	{"tpr_shadow", "Intel TPR Shadow"},
	{"vnmi", "Intel Virtual NMI"},
	{"flexpriority", "Intel FlexPriority"},
	{"ept", "Intel Extended Page Table"},
	{"vpid", "Intel Virtual Processor ID"},
	{"npt", "AMD Nested Page Table support"},
	{"lbrv", "AMD LBR Virtualization support"},
	{"svm_lock", "AMD SVM locking MSR"},
	{"nrip_save", "AMD SVM next_rip save"},
	{"tsc_scale", "AMD TSC scaling support"},
	{"vmcb_clean", "AMD VMCB clean bits support"},
	{"flushbyasid", "AMD flush-by-ASID support"},
	{"decodeassists", "AMD Decode Assists support"},
	{"pausefilter", "AMD filtered pause intercept"},
	{"pfthreshold", "AMD pause filter threshold"},
	/* extended intel */
	{"fsgsbase", "{RD/WR}{FS/GS}BASE instructions"},
	{"bmi1", "1st group bit manipulation extensions"},
	{"hle", "Hardware Lock Elision"},
	{"avx2", "AVX2 instructions"},
	{"smep", "Supervisor Mode Execution Protection"},
	{"bmi2", "2nd group bit manipulation extensions"},
	{"erms", "Enhanced REP MOVSB/STOSB"},
	{"invpcid", "Invalidate Processor Context ID"},
	{"rtm", "Restricted Transactional Memory"},
	{"mpx", "Memory Protection Extension"},
	{"rdseed", "The RDSEED instruction"},
	{"adx", "The ADCX and ADOX instructions"},
	{"smap", "Supervisor Mode Access Prevention"},
};

#define OPTSZ (sizeof(tbl)/sizeof(tbl[0]))

static void cleanup(void)
{
	if (file) {
		fclose(file);
		file = NULL;
	}
}

static void cpustat(char *start)
{
	unsigned i;
	while (*start) {
		while (*start && isspace(*start)) ++start;
		if (!*start) break;
		for (i = 0; i < OPTSZ; ++i) {
			if (!strncmp(tbl[i].flag, start, strlen(tbl[i].flag)) && (!start[strlen(tbl[i].flag)] || isspace(start[strlen(tbl[i].flag)]))) {
				puts(tbl[i].desc);
				goto skip;
			}
		}
		printf("bad flag: %s", start);
	skip:
		while (*start && !isspace(*start)) ++start;
	}
}

int main(int argc, char **argv)
{
	int ret = 1;
	if (argc > 2) {
		fprintf(stderr, "usage: %s [cpuinfo]\n", argv[0]);
		goto fail;
	}
	if (argc == 2)
		cpupath = argv[1];
	file = fopen(cpupath, "r");
	if (!file) {
		perror(cpupath);
		goto fail;
	}
	while (fgets(buf, BUFSZ, file)) {
		if (strncmp(buf, "flags", strlen("flags")))
			continue;
		char *start;
		start = strchr(buf, ':');
		if (!start)
			continue;
		cpustat(start + 1);
		break;
	}
	ret = 0;
fail:
	cleanup();
	return ret;
}
