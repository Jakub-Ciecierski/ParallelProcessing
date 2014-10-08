import java.util.Random;


/**
 * Drinks coffee with sugar
 */
public class PhD extends Thread{
	
	private int id;
	
	public PhD(int id){
		this.id = id;
	}
	
	public void run() {
		try {
			for (int i = 0; i < Main.TEST_CASES; i++) {
				Random rand = new Random();
				
				Main.s_participant.acquire();
				//Main.s_professor.
				/* critical section */

				System.out.println("***** PhD: " + id + " came to table *****");

				/*
				 * in case that in between refilling and waking up
				 * somebody else took the products, we have to check
				 * if the products are available
				 */
				while (!productsAvailable()) {
					System.out.println("PhD: " + id
							+ " waiting for his products...");

					// let other participants look for products
					Main.s_participant.release();
					
					// wait for waiter
					Main.s_refill.acquire();

					Main.s_participant.acquire();
				}

				// now that we know that all the products are available, take them
				takeProducts();

				System.out.println("***** PhD: " + id + " took his products *****");
				Main.printState();

				Thread.sleep(rand.nextInt(Main.PHD_BREAK_MAX - Main.PHD_BREAK_MIN) + Main.PHD_BREAK_MIN);

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
		if (Main.coffee <= 0 || Main.sugar <= 0) {
			return false;
		}
		return true;
	}

	/**
	 * Take the products
	 */
	private void takeProducts() {
		Main.coffee--;
		Main.sugar--;
	}
}
