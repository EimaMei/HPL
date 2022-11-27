bool functionTest() {
    string str = "s"

    if 30 > 50 || 30 == 320 || str == "if statement" {
        print("This gets printed because str equals to \"if statement\"")
        if true {
            print("This obviously gets printed as true equals to... true")

            if 4 == 2 {
                print("Doesn't get printed as 4 == 2 is false")
            }

            if false {
                print("Same story here")
            }

            if 30 < 40 {
                print("However, this gets printed!")
                str = "We also change the value of the variable cuz we can"
            }
        }
        print(f"Output: {str}")
    }

    return true
    print("This should not get printed")
}

print(functionTest()) // Bug: returns false if the `if` statement is false