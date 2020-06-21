import java.util.*;

public class sort {
	
	public static void main(String[] args) {
		int array[] = {2069, 1212, 2296, 2800, 544, 1618, 356, 1523, 4965, 3681};
		Arrays.sort(array);
		for (int i = 0; i < array.length; i++) {
			System.out.print(array[i] + " ");
		}
		System.out.println();
	}
}
