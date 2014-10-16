import java.util.Random;

/**
 * Drinks coffee and milk
 */
public class Doctor extends Thread{
	private int id;
	
	public Doctor(int id){
		this.id = id;
	}
	
	public void run() {
		try {
			for (int i = 0; i < Main.TEST_CASES; i++) {
				Random rand = new Random();

				System.out.println("***** Doctor: " + id + " came to table *****");
				
				// announces that he is interested in taking products
				Main.s_table.acquire();
				Main.doctor_queue_counter++;
				releaseParticipants();
				System.out.println("***** PhD: " + id + " announced interest in products *****");
				Main.s_table.release();
				
				// ask the waiter if he can get the products
				Main.s_doctor.acquire();
				
				System.out.println("***** Doctor: " + id + " after s_doctor.acquire() *****");
	
				// only one person can access the table at a time
				Main.s_table.acquire();
				/** critical section of a table **/
				System.out.println("***** Doctor: " + id + " after s_table.acquire() *****");
				
				try {
					takeProducts();
				} catch (ProductException e) {
					// ProductException tests if somebody took the products when
					// he shouldn't have
					e.printStackTrace();
					continue;
				}
				System.out.println("***** Doctor: " + id + " took his products *****");
				Main.printState();

				/** End of critical section of a table **/
				Main.s_table.release();

				Thread.sleep(rand.nextInt(Main.DOCTOR_BREAK_MAX
						- Main.DOCTOR_BREAK_MIN)
						+ Main.DOCTOR_BREAK_MIN);
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Checks if the participant can take his products
	 */
	static public boolean productsAvailable() {
		if (Main.coffee <= 0 || Main.milk <= 0 ) {
			return false;
		}
		return true;
	}

	/**
	 * Take the products
	 * @throws ProductException 
	 */
	private void takeProducts() throws ProductException {
		if (!productsAvailable()) {
			throw new ProductException("Products has been stolen by different process");
		}
		// refer to definitions for documentation of virtual consumption
		Main.virtual_coffee_consumption--;
		Main.virtual_milk_consumption--;

		Main.coffee--;
		Main.milk--;
	}
	
	/**
	 * Checks if awaiting participants are allowed
	 * to be released to take their products
	 */
	private void releaseParticipants() {
		/*
		 * Whenever a participant is waken up by a waiter he has to
		 * wait for the waiter to finish checking every
		 * set of products availability to release the main
		 * s_table semaphore.
		 * 
		 * Thus if a waiter woke somebody up, further checking of products
		 * has to include the fact that some will be missing.
		 * 
		 * (Main.coffee - Main.virtual_coffee_consumption) shows the true count
		 * of coffee in the system. 
		 * 
		 */
		for (int i = 0; i < Main.PROFESSOR_COUNT; i++) {
			if (Main.professor_queue_counter > 0
					&& (Main.coffee - Main.virtual_coffee_consumption) > 0
					&& (Main.milk - Main.virtual_milk_consumption) > 0
					&& (Main.sugar - Main.virtual_sugar_consumption) > 0) {
				// remove the participant from the queue counter
				Main.professor_queue_counter--;
				// release him from the queue
				Main.s_professor.release();

				System.out.println("Doctor: " + id + " waking up a professor");

				// keep track of the virtual consumption
				Main.virtual_coffee_consumption++;
				Main.virtual_milk_consumption++;
				Main.virtual_sugar_consumption++;
			}

		}

		for (int i = 0; i < Main.DOCTOR_COUNT; i++) {
			if (Main.doctor_queue_counter > 0 && Main.coffee - Main.virtual_coffee_consumption > 0
					&& Main.milk - Main.virtual_milk_consumption > 0) {
				Main.doctor_queue_counter--;
				Main.s_doctor.release();
				System.out.println("Doctor: " + id + " waking up a doctor");

				Main.virtual_coffee_consumption++;
				Main.virtual_milk_consumption++;
			}

		}

		for (int i = 0; i < Main.PHD_COUNT; i++) {
			if (Main.phd_queue_counter > 0 && Main.coffee - Main.virtual_coffee_consumption> 0
					&& Main.sugar - Main.virtual_sugar_consumption > 0) {
				Main.phd_queue_counter--;
				Main.s_phd.release();
				System.out.println("Doctor: " + id + " waking up a phd");

				Main.virtual_coffee_consumption++;
				Main.virtual_sugar_consumption++;
			}
		}

		for (int i = 0; i < Main.STUDENT_COUNT; i++) {
			if (Main.student_queue_counter > 0 && Main.milk - Main.virtual_milk_consumption > 0
					&& Main.sugar - Main.virtual_sugar_consumption > 0) {
				Main.student_queue_counter--;
				Main.s_student.release();
				System.out.println("Doctor: " + id + " waking up a student");
				
				Main.virtual_milk_consumption++;
				Main.virtual_sugar_consumption++;
			}
		}
	}
}
