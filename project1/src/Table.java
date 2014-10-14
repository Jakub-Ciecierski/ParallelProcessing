/**
 * Can be used instead or together with waiters
 */
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
		 * has to include the fact that some will be missing.
		 * 
		 * (Main.coffee - Main.virtual_coffee_consumption) shows the true count
		 * of coffee in the system. 
		 * 
		 */
		for (int i = 0; i < Main.PROFESSOR_COUNT; i++) {
			// buffer[i] equal to 1 means that the participant
			// awaits to take his products
			if (Main.professor_queue[i] == 1
					&& (Main.coffee - Main.virtual_coffee_consumption) > 0
					&& (Main.milk - Main.virtual_milk_consumption) > 0
					&& (Main.sugar - Main.virtual_sugar_consumption) > 0) {
				Main.professor_queue[i] = 0;
				Main.s_professor.release();
				System.out.println("Table waking up a professor");
				
				Main.virtual_coffee_consumption++;
				Main.virtual_milk_consumption++;
				Main.virtual_sugar_consumption++;
			}

		}

		for (int i = 0; i < Main.DOCTOR_COUNT; i++) {
			if (Main.doctor_queue[i] == 1 && Main.coffee - Main.virtual_coffee_consumption > 0
					&& Main.milk - Main.virtual_milk_consumption > 0) {
				Main.doctor_queue[i] = 0;
				Main.s_doctor.release();
				System.out.println("Table waking up a doctor");

				Main.virtual_coffee_consumption++;
				Main.virtual_milk_consumption++;
			}

		}

		for (int i = 0; i < Main.PHD_COUNT; i++) {
			if (Main.phd_queue[i] == 1 && Main.coffee - Main.virtual_coffee_consumption> 0
					&& Main.sugar - Main.virtual_sugar_consumption > 0) {
				Main.phd_queue[i] = 0;
				Main.s_phd.release();
				System.out.println("Table waking up a phd");

				Main.virtual_coffee_consumption++;
				Main.virtual_sugar_consumption++;
			}
		}

		for (int i = 0; i < Main.STUDENT_COUNT; i++) {
			if (Main.student_queue[i] == 1 && Main.milk - Main.virtual_milk_consumption > 0
					&& Main.sugar - Main.virtual_sugar_consumption > 0) {
				Main.student_queue[i] = 0;
				Main.s_student.release();
				System.out.println("Table waking up a student");
				
				Main.virtual_milk_consumption++;
				Main.virtual_sugar_consumption++;
			}
		}
	}
}
