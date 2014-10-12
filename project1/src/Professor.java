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
				
				Main.s_table.acquire();
				//Main.s_professor.
				/* critical section */

				System.out.println("***** Professor: " + id + " came to table *****");

				/*
				 * in case that in between refilling and waking up
				 * somebody else took the products, we have to check
				 * if the products are available
				 */
				if (!productsAvailable()) {
					System.out.println("Professor: " + id
							+ " waiting for his products...");

					// let other participants look for products
					Main.s_table.release();

					// wait for waiter
					Main.s_professor.acquire();

					Main.s_table.acquire();
				}
				
				// now that we know that all the products are available, take them
				try {
					takeProducts();
				} catch (ProductException e) {
					e.printStackTrace();
				}

				releaseParticipants();
				
				System.out.println("***** Professor: " + id + " took his products *****");
				Main.printState();

				Thread.sleep(rand.nextInt(Main.PROFESSOR_BREAK_MAX - Main.PROFESSOR_BREAK_MIN) + Main.PROFESSOR_BREAK_MIN);

				Main.s_table.release();

				/* End of critical section */
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
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
		 * has to include the fact that some will be missing 
		 * 
		 */
		int coffee_count = 0;
		int milk_count = 0;
		int sugar_count = 0;

		if (Main.s_doctor.getQueueLength() > 0) {
			if (Main.coffee - coffee_count > 0 && Main.milk - milk_count > 0) {
				Main.s_doctor.release();
				System.out.println("Professor: " + id + " waking up a doctor");
				
				coffee_count++;
				milk_count++;
			}
		}
		if (Main.s_phd.getQueueLength() > 0) {
			if (Main.coffee - coffee_count > 0 && Main.sugar - sugar_count > 0) {
				Main.s_phd.release();
				System.out.println("Professor: " + id + " waking up a phd");
				
				coffee_count++;
				sugar_count++;
			}
		}
		if (Main.s_student.getQueueLength() > 0) {
			if (Main.milk - milk_count > 0 && Main.sugar - sugar_count > 0) {
				Main.s_student.release();
				System.out.println("Professor: " + id + " waking up a student");
			}
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
	 */
	private void takeProducts() throws ProductException{
		if (!productsAvailable()) {
			throw new ProductException("Products has been stolen by different process");
		}
		Main.coffee--;
		Main.milk--;
		Main.sugar--;
	}
}
