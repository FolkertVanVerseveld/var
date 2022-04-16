// public domain

/**
 * Simple test to observe how JVM Garbage Collection with circular references.
 * @author Folkert van Verseveld
 */
public final class GCTest {
	GCTest ref;

	GCTest(GCTest ref) {
		this.ref = ref;
	}

	public static final void printMemoryStatistics() {
		Runtime rt = Runtime.getRuntime();

		// fetch state once as it may change during the format call
		long free = rt.freeMemory(), alloc = rt.totalMemory(), max = rt.maxMemory();

		System.out.format(
			"free memory      : %d KB\n" +
			"allocated memory : %d KB\n" +
			"max memory       : %d KB\n" +
			"total free memory: %d KB\n",
			free / 1024L, alloc / 1024L, max / 1024L, (free + max - alloc) / 1024L);
	}

	public static void main(String args[]) {
		printMemoryStatistics();

		// create a lot of circular references and release them
		for (long l = 0L; l < 1073741824L; ++l) {
			GCTest a = new GCTest(null);
			GCTest b = new GCTest(a);
			a.ref = b;
			a = null;
			b = null;
		}

		printMemoryStatistics();
	}
}

