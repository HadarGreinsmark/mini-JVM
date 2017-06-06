public class HelloWorld {

    public static int num = 0;

    public static void main(String[] args) {
        System.out.println("Hello");
        System.out.println("World");
        System.out.println(3);
        System.out.println(num);

        do{
            System.out.println("Branch-in");
            //num = num -1;
        }while(num==0);
    }

}
