import java.util.Scanner;

public class main {
	String dot = ".";
	char fSelect;
	boolean flag = true;
	double num1, num2, result;

	public void getInput() {
		// get input from user
		System.out.println("Please input your phase:");
		Scanner scan = new Scanner(System.in);
		String input = scan.nextLine();

		// define limited var
		char[] inputArray = input.toCharArray();
		char[] number1 = new char[inputArray.length];
		char[] number2 = new char[inputArray.length];
		// this is the end of the first number
		int end = 0;

		for (int i = 0; i < inputArray.length; i++) {
			// set first number
			if (flag == true) {
				if (Character.isDigit(inputArray[i]) || inputArray[i] == dot.charAt(0)) {
					number1[i] = inputArray[i];
					continue;
				}
				fSelect = inputArray[i];
				flag = false;
				end = i;
				// set index of the end of the number1
				// set flag to false to get number2
				continue;
			}
			number2[i - end - 1] = inputArray[i];

		}

		// convert char array to double
		String n1 = String.valueOf(number1);
		num1 = Double.parseDouble(n1);
		String n2 = String.valueOf(number2);
		num2 = Double.parseDouble(n2);

		// choose functions
		switch (String.valueOf(this.fSelect)) {
		case "+":
			result = num1 + num2;
			break;
		case "-":
			result = num1 - num2;
			break;
		case "*":
			result = num1 * num2;
			break;
		case "/":
			result = num1 / num2;
		}
	}

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		main m = new main();

		m.getInput();
		System.out.println(m.result);

	}

}
