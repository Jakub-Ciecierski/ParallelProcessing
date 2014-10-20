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
				Main.releaseParticipants();
				System.out.println("***** PhD: " + id + " announced interest in products *****");
				Main.s_table.release();
				
				// enter the queue
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
		// refer to Main.java for documentation of virtual consumption
		Main.virtual_coffee_consumption--;
		Main.virtual_milk_consumption--;

		Main.coffee--;
		Main.milk--;
	}
}
