// public domain
import java.lang.reflect.*;
/**
 * Break the integer cache
 * @author Folkert van Verseveld
 */
public class BrokenAdd {
	public static void main(String[] args) throws Exception {
		try {
			Field value = Integer.class.getDeclaredField("value");
			value.setAccessible(true);
			value.set(42, 43);
			System.out.printf("6 * 7 = %d\n", 6 * 7);
		} catch (NoSuchFieldException e) {
			System.err.println("don't know how to access internal value");
		} catch (Exception e) {
			throw e;
		}
	}
}
