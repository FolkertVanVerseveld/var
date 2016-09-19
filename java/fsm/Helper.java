// public domain
package fsm;

import java.util.*;

/**
 * Created by methos on 11-12-15.
 */
public class Helper<T> {
	// in c++11 we have initialiser lists, but not in
	// java because it probably is too complicated...
	public HashSet<T> wrap(T... list) {
		HashSet<T> set = new HashSet<T>();
		for (T e : list)
			set.add(e);
		return set;
	}
}
