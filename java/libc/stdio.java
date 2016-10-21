import java.io.*;
import java.util.*;

public class stdio {
	public static final InputStream stdin = System.in;
	public static final PrintStream stdout = System.out;
	public static final PrintStream stderr = System.err;
	public static final Scanner scnin = new Scanner(stdin);

	public static void putchar(int i) {
		stdout.print((char)i);
	}

	public static int getchar() {
		return scnin.nextByte();
	}

	public static void fputc(int i, PrintStream f) {
		f.print((char)i);
	}

	public static int fgetc(InputStream f) {
		return new Scanner(f).nextByte();
	}

	public static int fgetc(Scanner scn) {
		return scn.nextByte();
	}

	public static void puts(Object o) {
		stdout.println(o);
	}

	public static void fputs(Object o, PrintStream f) {
		f.print(o);
	}

	public static void printf(String format, Object... args) {
		stdout.printf(format, args);
	}

	public static void fprintf(PrintStream f, String format, Object... args) {
		f.printf(format, args);
	}

	public static int sprintf(StringBuilder sb, String f, Object... args) {
		String arg = String.format(f, args);
		sb.append(arg);
		return arg.length();
	}

	public static int snprintf(StringBuilder sb, int n, String f, Object... args) {
		String arg = String.format(f, args);
		if (arg.length() > n) {
			sb.append(arg.substring(0, n));
			return n;
		}
		sb.append(arg);
		return arg.length();
	}

	public static void main(String args[]) {
		puts("hello");
		fputs("world\n", stdout);
		printf("yay %s\n", "gay");
		putchar('!');
		fputc('\n', stdout);
		int a = 3, b = 2, c = a + b;
		fprintf(stdout, "%d + %d = %d\n", a, b, c);
	}
}
