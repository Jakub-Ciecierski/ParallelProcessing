import java.util.Random;

public class Waiter extends Thread {

	private int id;

	public Waiter(int id) {
		this.id = id;
	}

	public void run() {
		try {
			for(int i = 0;i < Main.TEST_CASES * 50; i++) {
				Random rand = new Random();
				
				Main.s_table.acquire();

				/* critical section */
	
				System.out.println("***** Waiter: " + id + " came to table *****");

				// refill to full a random product
				if (!refill()) {
					// if nothing to refill, go for a break
					System.out.println("Waiter: " + id + " nothing to refill");
				} else {
					Main.printState();
				}
				releaseParticipants();
				/* end of critical section */
				
				Main.s_table.release();

				Thread.sleep(rand.nextInt(Main.WAITER_BREAK_MAX - Main.WAITER_BREAK_MIN) + Main.WAITER_BREAK_MIN);
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
			System.out.println();
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

		if (Professor.productsAvailable()) {
			if( Main.s_professor.getQueueLength() > 0) {
				Main.s_professor.release();
				System.out.println("Waiter: " + id + " waking up a professor");

				coffee_count++;
				milk_count++;
				sugar_count++;
			}
		}
		if (Main.s_doctor.getQueueLength() > 0) {
			if (Main.coffee - coffee_count > 0 && Main.milk - milk_count > 0) {
				Main.s_doctor.release();
				System.out.println("Waiter: " + id + " waking up a doctor");
				
				coffee_count++;
				milk_count++;
			}
		}
		if (Main.s_phd.getQueueLength() > 0) {
			if (Main.coffee - coffee_count > 0 && Main.sugar - sugar_count > 0) {
				Main.s_phd.release();
				System.out.println("Waiter: " + id + " waking up a phd");
				
				coffee_count++;
				sugar_count++;
			}
		}
		if (Main.s_student.getQueueLength() > 0) {
			if (Main.milk - milk_count > 0 && Main.sugar - sugar_count > 0) {
				Main.s_student.release();
				System.out.println("Waiter: " + id + " waking up a student");
			}
		}
	}
	
	/**
	 * Refills a random product back to full
	 * 
	 * @return True if anything was refilled, false otherwise
	 */
	private boolean refill() {
		// first check if anything needs refilling
		boolean coffeeToFill = false;
		boolean milkToFill = false;
		boolean sugarToFill = false;

		if (Main.coffee != Main.COFFEE_COUNT) {
			coffeeToFill = true;
		}
		if (Main.milk != Main.MILK_COUNT) {
			milkToFill = true;
		}
		if (Main.sugar != Main.SUGAR_COUNT) {
			sugarToFill = true;
		}

		// refill a random non-full product
		if (coffeeToFill || milkToFill || sugarToFill) {
			while (true) {
				Random rnd = new Random();
				// int literal 3 - number of products
				int toFill = rnd.nextInt(3);

				switch (toFill) {
				case 0: // coffee
					if (!coffeeToFill) {
						break;
					} else {
						Main.coffee = Main.COFFEE_COUNT;
						System.out.println("Wainter: " + id
								+ " refilled coffee");
						return true;
					}
				case 1: // milk
					if (!milkToFill) {
						break;
					} else {
						Main.milk = Main.MILK_COUNT;
						System.out.println("Wainter: " + id + " refilled milk");
						return true;
					}
				case 2: // sugar
					if (!sugarToFill) {
						break;
					} else {
						Main.sugar = Main.SUGAR_COUNT;
						System.out
								.println("Wainter: " + id + " refilled sugar");
						return true;
					}

				}
			}
		}
		return false;
	}
}
