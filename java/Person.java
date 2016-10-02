/*
license: public domain
Example that uses java 8 lambda's.
*/
import java.time.LocalDate;
import java.util.ArrayList;

public class Person {
	public static final LocalDate now = LocalDate.now();

	public enum Sex {
		MALE, FEMALE
	}

	public final String name;
	public final LocalDate birthday;
	public final Sex gender;
	public final String emailAddress;

	public Person(String name, String bdate, Sex gender, String email) {
		this.name = name;
		birthday = LocalDate.parse(bdate);
		this.gender = gender;
		this.emailAddress = email;
	}

	public int getAge() {
		return now.getYear() - birthday.getYear();
	}

	public String toString() {
		return String.format("%s,%s born on %s, mail:%s", name, gender, birthday, emailAddress);
	}

	public static void puts(Object o) {
		System.out.println(o.toString());
	}

	public static void printf(Object o, Object... args) {
		System.out.format(o.toString(), args);
	}

	public static void main(String args[]) {
		ArrayList<Person> pp = new ArrayList<>();
		Person link;
		pp.add(new Person("King Harkinian", "1935-04-13", Sex.MALE, "maiboi@harkinian.gov"));
		pp.add(link = new Person("Link", "1987-07-10", Sex.MALE, "ohboy@harkinian.gov"));
		pp.add(new Person("Zelda", "1988-08-14", Sex.FEMALE, "father@harkinian.gov"));
		pp.add(new Person("Ganon", "1870-02-01", Sex.MALE, "youmustdie@itburns.com"));
		puts("persons:");
		for (Person p : pp)
			puts(p);
		int age = link.getAge();
		printf("\npeople that are at least %d years old:\n", age);
		for (Person p : pp) {
			if (p.getAge() >= age)
				puts(p);
		}
		int lower = age - 5, upper = age;
		printf("\npeople age in range [%d,%d]:\n", lower, upper);
		for (Person p : pp) {
			int a = p.getAge();
			if (a >= lower && a <= upper)
				puts(p);
		}
		puts("\nelderly male people:");
		CheckPersonElderlyMale elderFilter = new CheckPersonElderlyMale();
		for (Person p : pp) {
			if (elderFilter.test(p))
				puts(p);
		}
		puts("\nfemales:");
		filter(pp, new CheckPerson() {
			public boolean test(Person p) {
				return p.gender == Sex.FEMALE;
			}
		});
		puts("\npersons again:");
		// lambda magic
		pp.forEach(p -> puts(p));
		puts("\nmales:");
		pp.stream().filter(p -> p.gender == Sex.MALE).forEach(p -> puts(p));
	}

	static interface CheckPerson {
		boolean test(Person p);
	}

	static class CheckPersonElderlyMale implements CheckPerson {
		public boolean test(Person p) {
			return p.getAge() >= 65 && p.gender == Sex.MALE;
		}
	}

	public static void filter(ArrayList<Person> pp, CheckPerson filter) {
		for (Person p : pp) {
			if (filter.test(p))
				puts(p);
		}
	}
}
