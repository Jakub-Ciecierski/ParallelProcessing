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
				
				Main.s_participant.acquire();
				//Main.s_professor.
				/* critical section */

				System.out.println("***** Doctor: " + id + " came to table *****");

				/*
				 * in case that in between refilling and waking up
				 * somebody else took the products, we have to check
				 * if the products are available
				 */
				while (!productsAvailable()) {
					System.out.println("Doctor: " + id
							+ " waiting for his products...");

					// let other participants look for products
					Main.s_participant.release();
					
					// wait for waiter
					Main.s_refill.acquire();

					Main.s_participant.acquire();
				}

				// now that we know that all the products are available, take them
				takeProducts();

				System.out.println("***** Doctor: " + id + " took his products *****");
				Main.printState();

				Thread.sleep(rand.nextInt(Main.DOCTOR_BREAK_MAX - Main.DOCTOR_BREAK_MIN) + Main.DOCTOR_BREAK_MIN);

				Main.s_participant.release();

				/* End of critical section */
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Checks if the participant can take his products
	 */
	private boolean productsAvailable() {
		if (Main.coffee <= 0 || Main.milk <= 0 ) {
			return false;
		}
		return true;
	}

	/**
	 * Take the products
	 */
	private void takeProducts() {
		Main.coffee--;
		Main.milk--;
	}
}
