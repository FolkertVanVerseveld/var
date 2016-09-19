// public domain
import java.io.PrintStream;
import java.text.NumberFormat;

/**
Simple test that creates *a lot* of circular references.
If memory keeps growing, this may indicate that the JVM
can not release the objects because the references are
circular.

The code looks weird because I lost the source and had to decompile it :p
*/
public final class GCTest {
	GCTest ref;

	GCTest(GCTest paramGCTest) {
		this.ref = paramGCTest;
	}

	public static final void dumpStuff() {
		Runtime runtime = Runtime.getRuntime();
		NumberFormat nf = NumberFormat.getInstance();
		StringBuilder sb = new StringBuilder();
		long l1 = runtime.maxMemory();
		long l2 = runtime.totalMemory();
		long l3 = runtime.freeMemory();
		sb.append("free memory: " + nf.format(l3 / 1024L) + "\n");
		sb.append("allocated memory: " + nf.format(l2 / 1024L) + "\n");
		sb.append("max memory: " + nf.format(l1 / 1024L) + "\n");
		sb.append("total free memory: " + nf.format((l3 + (l1 - l2)) / 1024L) + "\n");
		System.out.println(sb.toString());
	}

	public static final void main(String[] paramArrayOfString) {
		for (long l = 0L; l < 1073741824L; l += 1L) {
			GCTest a = new GCTest(null);
			GCTest b = new GCTest(a);
			a.ref = b;
			a = null;
			b = null;
		}
		dumpStuff();
	}
}
