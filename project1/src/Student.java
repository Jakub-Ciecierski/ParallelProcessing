import java.util.Random;

/**
 * Drinks milk and sugar
 */
public class Student extends Thread{

	private int id;
	
	public Student(int id){
		this.id = id;
	}
	
	public void run() {
		try {
			for (int i = 0; i < Main.TEST_CASES; i++) {
				Random rand = new Random();

				System.out.println("***** Student: " + id + " came to table *****");

				// announces that he is interested in taking products
				Main.student_queue[this.id - 1] = 1;
				
				// ask the waiter if he can get the products
				Main.s_student.acquire();
				
				System.out.println("***** Student: " + id + " after s_student.acquire() *****");
				
				// only one person can access the table at a time
				Main.s_table.acquire();
				
				System.out.println("***** Student: " + id + " after s_table.acquire() *****");
				
				/** critical section of a table **/
				try {
					takeProducts();
				} catch (ProductException e) {
					// ProductException tests if somebody took the products when
					// he shouldn't have
					e.printStackTrace();
					continue;
				}
				System.out.println("***** Student: " + id + " took his products *****");
				Main.printState();
				
				/** End of critical section of a table**/
				Main.s_table.release();

				Thread.sleep(rand.nextInt(Main.STUDENT_BREAK_MAX
						- Main.STUDENT_BREAK_MIN)
						+ Main.STUDENT_BREAK_MIN);
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Checks if the participant can take his products
	 */
	static public boolean productsAvailable() {
		if (Main.milk <= 0 || Main.sugar <= 0) {
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
		Main.virtual_milk_consumption--;
		Main.virtual_sugar_consumption--;
		Main.milk--;
		Main.sugar--;
	}
}
