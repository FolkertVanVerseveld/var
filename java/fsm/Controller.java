// public domain
package fsm;

import java.util.*;

/**
 * Created by methos on 11-12-15.
 */
public class Controller<T> {
	protected T state;
	protected HashMap<T, HashSet<T>> transitions;
	protected HashSet<Listener<T>> callback;

	public Controller(T current, HashMap<T, HashSet<T>> changes, HashSet<Listener<T>> listeners) {
		state = current;
		transitions = changes;
		callback = listeners;
		enter(current);
	}

	protected void leave(T state) {
		for (Listener l : callback)
			l.leave(state);
	}

	protected void enter(T state) {
		for (Listener l : callback)
			l.enter(state);
	}

	public void gotoState(T next) {
		if (!transitions.containsKey(state)) {
			String oops = String.format("Dead end in state %s", state);
			throw new IllegalStateException(oops);
		}
		if (!transitions.get(state).contains(next)) {
			String oops = String.format("Cannot change state from %s to %s", state, next);
			throw new IllegalStateException(oops);
		}
		leave(state);
		state = next;
		enter(state);
	}
}
