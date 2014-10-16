import java.util.concurrent.Semaphore;

/**
 ***************************** Task description ******************************
 * Three products: Coffee, milk, sugar.
 * 
 * Each product has separate container.
 * Each container can hold at most P_COUNT of P product
 * 
 * WAITER_COUNT waiters come with random intervals: Refills a random non-full
 * container back to maximum.
 * 
 * Participants: 
 * Professors - coffee, milk and sugar.
 * Doctors - coffee and milk.
 * PhD students - coffee and sugar. 
 * Students - milk and sugar.
 * 
 * 1) They come for their products in random order.
 * 2) Random interval between any two participants.
 * 3) Each takes all needed products (or nothing at all) otherwise he waits. 
 * 4) After collecting products he goes to drink it.
 * 5) If all products are available Professors have priority.
 * 
 * **************************** Solution ******************************
 * Tools used in the solution:
 * 
 * Semaphores:
 * s_table - locks the access to the table (products).
 * s_professor, s_doctor, s_phd, s_student - lock the action of
 * consumption of each set of products.
 * 
 * 
 * Waiters can release participants' semaphores, by checking
 * if any of them are waiting for consumption (using participant_queue[])
 * and further save the virtual state of consumption 
 * using virtual_product_count counter to prevent two independent waiters,
 * who accessed the table before the freshly released 
 * participants, to release more participants
 * than the current product count allows. 
 * 
 */
public class Main {
	
	/*
	 * The lower and upper bound of participants and waiters' break.
	 * The actual break time will be a random integer in [lower,upper] interval.
	 * 
	 * Measured in mili seconds.
	 */
	public static final int WAITER_BREAK_MIN = 200;
	public static final int WAITER_BREAK_MAX = 600;

	public static final int PROFESSOR_BREAK_MIN = 1000;
	public static final int PROFESSOR_BREAK_MAX = 2000;

	public static final int DOCTOR_BREAK_MIN = 1000;
	public static final int DOCTOR_BREAK_MAX = 2000;

	public static final int PHD_BREAK_MIN = 1000;
	public static final int PHD_BREAK_MAX = 2000;

	public static final int STUDENT_BREAK_MIN = 1000;
	public static final int STUDENT_BREAK_MAX = 2000;
	
	/*
	 * Number of iteration of each thread
	 */
	public static final int TEST_CASES = 50;

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
	 * Semaphores:
	 * s_table locks the access to the table
	 * to only 1 person at a time.
	 * 
	 * s_x, where x is a participant type,
	 * locks the access to theirs set of products.
	 */
	public static Semaphore s_table;
	public static Semaphore s_professor;
	public static Semaphore s_doctor;
	public static Semaphore s_phd;
	public static Semaphore s_student;
	
	/*
	 * Containers, keep track of current count
	 */
	public static volatile int coffee;
	public static volatile int milk;
	public static volatile int sugar;
	
	/*
	 * participant_queue[]
	 * 
	 * Keep the track of the queues
	 * when waiting for missing products.
	 * 
	 * Example:
	 * x_queue[i] = 1 	if i-th x-participant
	 * 					is awaiting products.
	 * x_queue[i] = 0 	if i-th x-participant
	 * 					has been freed from waiting.
	 * 
	 * TODO Change it to simple counters
	 * and make sure that participant can access products
	 * from very beginning - independent of waiters
	 */
	public static int professor_queue[];
	public static int doctor_queue[];
	public static int phd_queue[];
	public static int student_queue[];
	
	/*
	 * virtual_product_consumption
	 * 
	 * Virtual consumption count tracks the amount of
	 * consumed products between waking participants
	 * and their actual consumption.
	 * 
	 * For example:
	 * Assume such state:
	 * coffee = 3, milk = 3, sugar = 0.
	 * 5 Professors awaiting their products in s_professor queue.
	 * 2 Waiters W1 and W2, come in to the table, W2 is waiting in s_table queue.
	 * W1 refills sugar and wakes up 3 Professors, then unlocks s_table for W2.
	 * W2 has to know that 3 Professors have been awaken.
	 * Thus the need for virtual consumption.
	 */
	public static int virtual_coffee_consumption = 0;
	public static int virtual_milk_consumption = 0;
	public static int virtual_sugar_consumption = 0;



	public static void main(String argv[]) {
		// fill the containers
		coffee = COFFEE_COUNT;
		milk = MILK_COUNT;
		sugar = SUGAR_COUNT;

		// binary semaphore for accessing the table
		s_table = new Semaphore(1);
		
		s_professor = new Semaphore(0);
		s_doctor = new Semaphore(0);
		s_phd = new Semaphore(0);
		s_student = new Semaphore(0);
		
		// fill queues with 0's (not waiting) for each participant
		professor_queue = new int[PROFESSOR_COUNT];
		doctor_queue = new int[DOCTOR_COUNT];
		phd_queue = new int[PHD_COUNT];
		student_queue = new int[STUDENT_COUNT];
		
		// Spawn participants to have some fun
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

	/**
	 * Prints the current state of the system.
	 */
	public static void printState() {
		System.out.println();
		System.out.println("******************* CURRENT STATE *******************");
		System.out.println("\t******* PRODUCTS *******");
		System.out.println("\t\tCOFFEE:  " + coffee);
		System.out.println("\t\tMILK:  " + milk);
		System.out.println("\t\tSUGAR: " + sugar);
		System.out.println("\t******* VIRTUAL CONSUMPTION *******");
		System.out.println("\t\tCOFFEE: " + virtual_coffee_consumption);
		System.out.println("\t\tMILK:   " + virtual_milk_consumption);
		System.out.println("\t\tSUGAR:  " + virtual_sugar_consumption);
		System.out.println("\t*******  QUEUES ******* ");
		System.out.println("\t\tPROFESSORS: " + s_professor.getQueueLength());
		System.out.println("\t\tDOCTORS:    " + s_doctor.getQueueLength());
		System.out.println("\t\tPHDS:       " + s_phd.getQueueLength());
		System.out.println("\t\tSTUDENTS:   " + s_student.getQueueLength());
		System.out.println("**************************************");
		System.out.println();
	}
}
