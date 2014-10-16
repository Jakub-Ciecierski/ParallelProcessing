import java.util.Random;

/**
 * Drinks coffee, milk and sugar
 */
public class Professor extends Thread {

	private int id;

	public Professor(int id) {
		this.id = id;
	}

	public void run() {
		try {
			for (int i = 0; i < Main.TEST_CASES; i++) {
				Random rand = new Random();

				System.out.println("***** Professor: " + id + " came to table *****");

				// announces that he is interested in taking products
				Main.s_table.acquire();
				Main.professor_queue_counter++;
				System.out.println("***** Professor: " + id + " announced interest in products *****");
				releaseParticipants();
				Main.s_table.release();
				
				// ask the waiter if he can get the products
				Main.s_professor.acquire();
				System.out.println("***** Professor: " + id + " after s_professor.acquire() *****");
				
				// only one person can access the table at a time
				Main.s_table.acquire();
				/** critical section of a table **/
				System.out.println("***** Professor: " + id + " after s_table.acquire() *****");

				try {
					takeProducts();
				} catch (ProductException e) {
					// ProductException tests if somebody took the products when
					// he shouldn't have
					e.printStackTrace();
					continue;
				}
				System.out.println("***** Professor: " + id + " took his products *****");
				Main.printState();

				/** End of critical section of a table **/
				Main.s_table.release();

				Thread.sleep(rand.nextInt(Main.PROFESSOR_BREAK_MAX
						- Main.PROFESSOR_BREAK_MIN)
						+ Main.PROFESSOR_BREAK_MIN);
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	

	/**
	 * Checks if the participant can take his products
	 */
	static public boolean productsAvailable() {
		if (Main.coffee <= 0 || Main.milk <= 0 || Main.sugar <= 0) {
			return false;
		}
		return true;
	}

	/**
	 * Take the products
	 * @throws ProductException 
	 */
	private void takeProducts() throws ProductException{
		if (!productsAvailable()) {
			throw new ProductException("Products has been stolen by different process");
		}
		// refer to definitions for documentation of virtual consumption
		Main.virtual_coffee_consumption--;
		Main.virtual_milk_consumption--;
		Main.virtual_sugar_consumption--;

		Main.coffee--;
		Main.milk--;
		Main.sugar--;
	}
	
	/**
	 * Checks if awaiting participants are allowed
	 * to be released to take their products
	 */
	private void releaseParticipants() {
		/*
		 * If somebody has been waken up, further checking of products
		 * has to include the fact that some will be missing.
		 * 
		 * (Main.coffee - Main.virtual_coffee_consumption) shows the true count
		 * of coffee in the system. 
		 * 
		 */
		// Professors have the priority provided that
		// they can access their products immediately.
		for (int i = 0; i < Main.PROFESSOR_COUNT; i++) {
			if (Main.professor_queue_counter > 0
					&& (Main.coffee - Main.virtual_coffee_consumption) > 0
					&& (Main.milk - Main.virtual_milk_consumption) > 0
					&& (Main.sugar - Main.virtual_sugar_consumption) > 0) {
				// remove the participant from the queue counter
				Main.professor_queue_counter--;
				// release him from the queue
				Main.s_professor.release();

				System.out.println("Professor: " + id + " waking up a professor");

				// keep track of the virtual consumption
				Main.virtual_coffee_consumption++;
				Main.virtual_milk_consumption++;
				Main.virtual_sugar_consumption++;
			}

		}
		// TODO create random selection of further participants
		
		for (int i = 0; i < Main.DOCTOR_COUNT; i++) {
			if (Main.doctor_queue_counter > 0 && Main.coffee - Main.virtual_coffee_consumption > 0
					&& Main.milk - Main.virtual_milk_consumption > 0) {
				Main.doctor_queue_counter--;
				Main.s_doctor.release();
				System.out.println("Professor: " + id + " waking up a doctor");

				Main.virtual_coffee_consumption++;
				Main.virtual_milk_consumption++;
			}

		}

		for (int i = 0; i < Main.PHD_COUNT; i++) {
			if (Main.phd_queue_counter > 0 && Main.coffee - Main.virtual_coffee_consumption> 0
					&& Main.sugar - Main.virtual_sugar_consumption > 0) {
				Main.phd_queue_counter--;
				Main.s_phd.release();
				System.out.println("Professor: " + id + " waking up a phd");

				Main.virtual_coffee_consumption++;
				Main.virtual_sugar_consumption++;
			}
		}

		for (int i = 0; i < Main.STUDENT_COUNT; i++) {
			if (Main.student_queue_counter > 0 && Main.milk - Main.virtual_milk_consumption > 0
					&& Main.sugar - Main.virtual_sugar_consumption > 0) {
				Main.student_queue_counter--;
				Main.s_student.release();
				System.out.println("Professor: " + id + " waking up a student");
				
				Main.virtual_milk_consumption++;
				Main.virtual_sugar_consumption++;
			}
		}
	}
}
