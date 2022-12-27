bool functionTest(string str) {
    if 30 > 50 || 30 == 320 || str == "if statement" {
        print("1This gets printed because str equals to \"if statement\"")
        if true {
            print("2This obviously gets printed as true equals to... true")

            if 4 == 2 {
                print("6Doesn't get printed as 4 == 2 is false")
            }

            if false {
                print("7Same story here")
            }

            if 30 < 40 {
                print("3However, this gets printed!")
                str = "We also change the value of the variable since we can"
            }
        }
        print(f"4Output: {str}")

        return true
    }

    return false
    print("8This should not get printed")
}

print("5The function 'functionTest' has outputted a '" + functionTest(str = "if statement") + "', not something else.")