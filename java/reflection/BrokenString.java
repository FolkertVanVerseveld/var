// public domain
import java.lang.reflect.*;
/**
 * Breaks how Strings work
 * @author Folkert van Verseveld
 */
public class BrokenString {
	public static void main(String[] args) throws Exception {
		try {
			Field value = String.class.getDeclaredField("value");
			value.setAccessible(true);
			value.set("hello", "goodbye".toCharArray());
			System.out.println("hello");
			System.out.printf("compare(%s,%s) == %d", "hello", "goodbye", "hello".compareTo("goodbye"));
		} catch (NoSuchFieldException e) {
			System.err.println("don't know how to access internal value");
		} catch (Exception e) {
			throw e;
		}
	}
}
