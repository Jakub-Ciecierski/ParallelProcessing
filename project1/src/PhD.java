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

				System.out.println("***** PhD: " + id + " came to table *****");
				
				// announces that he is interested in taking products
				Main.s_table.acquire();
				Main.phd_queue_counter++;
				Main.releaseParticipants();
				System.out.println("***** PhD: " + id + " announced interest in products *****");
				Main.s_table.release();
				
				// enter the queue
				Main.s_phd.acquire();
				System.out.println("***** PhD: " + id + " after s_phd.acquire() *****");
				
				// only one person can access the table at a time
				Main.s_table.acquire();
				/** critical section of a table **/
				System.out.println("***** PhD: " + id + " after s_table.acquire() *****");
				
				try {
					takeProducts();
				} catch (ProductException e) {
					// ProductException tests if somebody took the products when
					// he shouldn't have
					e.printStackTrace();
					continue;
				}
				System.out.println("***** PhD: " + id + " took his products *****");
				Main.printState();

				/** End of critical section of a table **/
				Main.s_table.release();

				Thread.sleep(rand.nextInt(Main.PHD_BREAK_MAX
						- Main.PHD_BREAK_MIN)
						+ Main.PHD_BREAK_MIN);
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Checks if the participant can take his products
	 */
	static public boolean productsAvailable() {
		if (Main.coffee <= 0 || Main.sugar <= 0) {
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
		// refer to Main.java for documentation of virtual consumption
		Main.virtual_coffee_consumption--;
		Main.virtual_sugar_consumption--;

		Main.coffee--;
		Main.sugar--;
	}

}
