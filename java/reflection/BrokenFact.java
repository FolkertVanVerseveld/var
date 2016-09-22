// public domain
import java.lang.reflect.*;
/**
 * Breaks boolean and breaks if behavior
 * @author Folkert van Verseveld
 */
public class BrokenFact {
	public static void main(String[] args) throws Exception {
		try {
			Field value = Boolean.class.getDeclaredField("FALSE");
			value.setAccessible(true);
			// only non-final fields can be changed
			Field mod = Field.class.getDeclaredField("modifiers");
			mod.setAccessible(true);
			mod.setInt(value, value.getModifiers() & ~Modifier.FINAL);
			// null represents static context
			value.set(null, true);
			if ((Boolean)(null != null))
				System.out.printf("everything is %s\n", false);
		} catch (NoSuchFieldException e) {
			System.err.println("don't know how to access internal value");
		} catch (Exception e) {
			throw e;
		}
	}
}
