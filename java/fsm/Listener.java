// public domain
package fsm;

/**
 * Created by methos on 11-12-15.
 */
public interface Listener<T> {
	void leave(T state);
	void enter(T state);
}
