import java.util.Random;

public class Waiter extends Thread {

	private int id;

	public Waiter(int id) {
		this.id = id;
	}

	public void run() {
		try {
			for(int i = 0;i < Main.TEST_CASES; i++) {
				Random rand = new Random();
				
				Main.s_waiter.acquire();

				/* critical section */
	
				System.out.println("***** Waiter: " + id + " came to table *****");
				// if nothing to refill, go for a break
				if (!refill()) {
					System.out.println("Waiter: " + id + " nothing to refill");
					Thread.sleep(Main.WAITER_BREAK_MAX);
				} else {
					Main.s_refill.release();
					System.out.println("s_refill permits: " + Main.s_refill.availablePermits());
					Main.printState();
				}
				Thread.sleep(rand.nextInt(Main.WAITER_BREAK_MAX - Main.WAITER_BREAK_MIN) + Main.WAITER_BREAK_MIN);

				/* end of critical section */
				
				Main.s_waiter.release();
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
			System.out.println();
		}
	}

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
