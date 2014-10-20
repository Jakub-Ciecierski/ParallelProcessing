import java.util.Random;
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
 * Participants and Waiters can release other (including themselves)
 * participants' semaphores, by checking
 * if any of them are waiting for consumption (using participant_queue)
 * and further saving the virtual state of consumption 
 * using virtual_product_count counter to prevent two independent participants,
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
	 * participant_queue
	 * 
	 * Keep the track of the queues
	 * when waiting for missing products.
	 */
	public static int professor_queue_counter;
	public static int doctor_queue_counter;
	public static int phd_queue_counter;
	public static int student_queue_counter;
	
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
		
		/*
		 *  Initiate semaphores with value 0.
		 *  Notice that each participant, before acquiring his semaphore
		 *  will release all participants waiting for missing products,
		 *  including himself, provided that the system 
		 *  allows him to (queue_count + virtual_consumption).
		 *  
		 *  Assume the scenario when a single Professor comes to the table.
		 *  First he will increase professor_queue_counter, and then
		 *  enter releaseParticipant() function which in fact will release himself.
		 *  Then he goes to acquire his semaphore 
		 *  (he doesn't wait since he's just released himself)
		 */
		s_professor = new Semaphore(0);
		s_doctor = new Semaphore(0);
		s_phd = new Semaphore(0);
		s_student = new Semaphore(0);
		
		// nobody is waiting in the queue at the start
		professor_queue_counter = 0;
		doctor_queue_counter = 0;
		phd_queue_counter = 0;
		student_queue_counter = 0;
		
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
	
	/**
	 * Checks if awaiting participants are allowed
	 * to be released to take their products
	 */
	public static void releaseParticipants() {
		/*
		 * If somebody has been waken up, further checking of products
		 * has to include the fact that some will be missing.
		 * 
		 * For example:
		 * (Main.coffee - Main.virtual_coffee_consumption) shows the true count
		 * of coffee in the system. 
		 */

		// Professors have the priority provided that
		// they can access their products immediately.
		releaseProfessor();

		// Serve other participants in random order
		Random rand = new Random();

		boolean done = true;
		int served[] = new int[3];

		while(done){
			int next = rand.nextInt(3);
			while(served[next] == 1){
				next = rand.nextInt(3);
			}
			switch(next){
				case 0:
					served[0] = 1;
					releaseDoctor();
					break;
				case 1:
					served[1] = 1;
					releasePhD();
					break;
				case 2:
					served[2] = 1;
					releaseStudent();
					break;
			}
			if(served[0] == 1
				&& served[1] == 1
				&& served[2] == 1){
				done = false;
			}
		}
	}

	private static void releaseProfessor(){
		while (Main.professor_queue_counter > 0
				&& (Main.coffee - Main.virtual_coffee_consumption) > 0
				&& (Main.milk - Main.virtual_milk_consumption) > 0
				&& (Main.sugar - Main.virtual_sugar_consumption) > 0) {
			// remove the participant from the queue counter
			Main.professor_queue_counter--;
			// release him from the queue
			Main.s_professor.release();

			System.out.println("Professor has been waken up");

			// keep track of the virtual consumption
			Main.virtual_coffee_consumption++;
			Main.virtual_milk_consumption++;
			Main.virtual_sugar_consumption++;
		}
	}
	
	private static void releaseDoctor(){
		while (Main.doctor_queue_counter > 0 
				&& Main.coffee - Main.virtual_coffee_consumption > 0
				&& Main.milk - Main.virtual_milk_consumption > 0) {
			Main.doctor_queue_counter--;
			Main.s_doctor.release();
			System.out.println("Doctor has been waken up");

			Main.virtual_coffee_consumption++;
			Main.virtual_milk_consumption++;
		}
	}
	
	private static void releasePhD(){
		while (Main.phd_queue_counter > 0 
				&& Main.coffee - Main.virtual_coffee_consumption> 0
				&& Main.sugar - Main.virtual_sugar_consumption > 0) {
			Main.phd_queue_counter--;
			Main.s_phd.release();
			System.out.println("PhD has been waken up");

			Main.virtual_coffee_consumption++;
			Main.virtual_sugar_consumption++;
		}
	}
	
	private static void releaseStudent(){
		while (Main.student_queue_counter > 0 
				&& Main.milk - Main.virtual_milk_consumption > 0
				&& Main.sugar - Main.virtual_sugar_consumption > 0) {
			Main.student_queue_counter--;
			Main.s_student.release();
			System.out.println("Student has been waken up");
			
			Main.virtual_milk_consumption++;
			Main.virtual_sugar_consumption++;
		}
	}
}
