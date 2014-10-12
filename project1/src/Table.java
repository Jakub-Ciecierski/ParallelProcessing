
public class Table extends Thread {
	public Table() {
		
	}

	public void run() {
		for(;;){
			try {
				Main.s_table.acquire();
				releaseParticipants();
				Main.s_table.release();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
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
				System.out.println("waking up a professor");

				coffee_count++;
				milk_count++;
				sugar_count++;
			}
		}
		if (Main.s_doctor.getQueueLength() > 0) {
			if (Main.coffee - coffee_count > 0 && Main.milk - milk_count > 0) {
				Main.s_doctor.release();
				System.out.println("waking up a doctor");
				
				coffee_count++;
				milk_count++;
			}
		}
		if (Main.s_phd.getQueueLength() > 0) {
			if (Main.coffee - coffee_count > 0 && Main.sugar - sugar_count > 0) {
				Main.s_phd.release();
				System.out.println("waking up a phd");
				
				coffee_count++;
				sugar_count++;
			}
		}
		if (Main.s_student.getQueueLength() > 0) {
			if (Main.milk - milk_count > 0 && Main.sugar - sugar_count > 0) {
				Main.s_student.release();
				System.out.println("waking up a student");
			}
		}
	}
}
