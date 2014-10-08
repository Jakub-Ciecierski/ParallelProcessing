import java.util.concurrent.Semaphore;

/**
 * Three products: Coffee, milk, sugar.
 * 
 * Each product has separate container. Each container can hold at most P_COUNT
 * of P product
 * 
 * WAITER_COUNT waiters with random intervals: Refills a random non-full
 * container back to max. If all containers are full waiter goes for a break -
 * random time.
 * 
 * Participants: Professors - coffee, milk and sugar. Doctors - coffee and milk
 * PhD students - coffee and sugar Students - milk and sugar.
 * 
 * 1) They come for their products in random order. 2) Random interval between
 * any two participants. 3) Each takes all needed products (or nothing at all)
 * otherwise he waits. 4) After collecting products he goes to drink it - random
 * interval (TO BE DISCUSSED). 5) If all products are available and at least one
 * Professor is waiting, no other participant will start collecting products.
 * 
 */
public class Main {

	public static final int TEST_CASES = 5;

	/*
	 * The maximum count of products in their containers.
	 */
	public static final int COFFEE_COUNT = 3;
	public static final int MILK_COUNT = 3;
	public static final int SUGAR_COUNT = 3;

	/*
	 * Number of waiters and participants.
	 */
	public static final int WAITER_COUNT = 2;
	public static final int PROFESSOR_COUNT = 5;
	public static final int DOCTOR_COUNT = 5;
	public static final int PHD_COUNT = 5;
	public static final int STUDENT_COUNT = 5;

	/*
	 * The lower and upper bound of participants and waiters' break The actual
	 * break time will be a random integer in [lower,upper] interval.
	 * 
	 * Measured in miliseconds.
	 */
	public static final int WAITER_BREAK_MIN = 2000;
	public static final int WAITER_BREAK_MAX = 6000;

	public static final int PROFESSOR_BREAK_MIN = 1000;
	public static final int PROFESSOR_BREAK_MAX = 2000;

	public static final int DOCTOR_BREAK_MIN = 1000;
	public static final int DOCTOR_BREAK_MAX = 2000;

	public static final int PHD_BREAK_MIN = 1000;
	public static final int PHD_BREAK_MAX = 2000;

	public static final int STUDENT_BREAK_MIN = 1000;
	public static final int STUDENT_BREAK_MAX = 2000;

	/*
	 * Containers
	 */
	public static volatile int coffee;
	public static volatile int milk;
	public static volatile int sugar;

	/*
	 * 
	 */
	public static Semaphore s_waiter;
	public static Semaphore s_participant;
	public static Semaphore s_refill;
	public static Semaphore s_professor;

	public static void main(String argv[]) {
		coffee = COFFEE_COUNT;
		milk = MILK_COUNT;
		sugar = SUGAR_COUNT;

		s_waiter = new Semaphore(1);
		s_participant = new Semaphore(1);
		s_refill = new Semaphore(0);
		s_professor = new Semaphore(1);

		for (int i = 0; i < PROFESSOR_COUNT; i++) {
			Professor p = new Professor(i + 1);
			p.start();
		}
		
		for (int i = 0; i < DOCTOR_COUNT; i++) {
			Doctor d = new Doctor(i + 1);
			d.start();
		}
		
		for (int i = 0; i < PHD_COUNT; i++) {
			PhD ph = new PhD(i + 1);
			ph.start();
		}
		
		for (int i = 0; i < STUDENT_COUNT; i++) {
			Student s = new Student(i + 1);
			s.start();
		}
		
		for (int i = 0; i < WAITER_COUNT; i++) {
			Waiter w = new Waiter(i+1);
			w.start();
		}
	}

	public static void printState() {
		System.out.println();
		System.out.println("****CURRENT STATE****");
		System.out.println("COFFEE:" + coffee);
		System.out.println("MILK:" + milk);
		System.out.println("SUGAR:" + sugar);
		System.out.println();
	}
}
