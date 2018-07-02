// types of tests provided by the developers (can be found at the CMakeLists.txt files)
enum TestTypes {
	module(0), integration(1), system(2), cmake(3), regression(4), benchmark(5), framework(6)
	TestTypes(int order) {
		this.order = order
	}
	private final int order
	int getOrder() {
		order
	}
}
return this
