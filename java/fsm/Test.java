// public domain
package fsm;

import java.util.*;

/**
 * Created by methos on 11-12-15.
 */

public class Test {
	public enum State {
		START,
		FIRE,
		TIMEOUT,
		ACCEPT,
		END;
	}

	private static final HashMap<State, HashSet<State>> FIRE_STATES;

	static {
		Helper<State> helper = new Helper<State>();
		HashMap<State, HashSet<State>> map = new HashMap<State, HashSet<State>>();
		map.put(State.START, helper.wrap(State.FIRE));
		map.put(State.FIRE, helper.wrap(State.ACCEPT, State.TIMEOUT));
		map.put(State.ACCEPT, helper.wrap(State.END));
		map.put(State.TIMEOUT, helper.wrap(State.END));
		FIRE_STATES = map;
	}

	public static void main(String args[]) {
		Listener dummyListener = new Listener<State>() {
			@Override
			public void leave(State state) {
				System.out.printf("leave: %s\n", state);
			}

			@Override
			public void enter(State state) {
				System.out.printf("enter: %s\n", state);
			}
		};
		Helper<Listener<State>> helper = new Helper<Listener<State>>();
		Controller<State> fire = new Controller<State>(
			State.START, FIRE_STATES,
			helper.wrap(dummyListener)
		);
		fire.gotoState(State.FIRE);
		fire.gotoState(State.TIMEOUT);
		fire.gotoState(State.END);
	}
}
