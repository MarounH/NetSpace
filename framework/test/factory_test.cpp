#include <iostream>

#include "factory.hpp"
using namespace hrd31;

class Shape
{
public:
	virtual void Draw(int times_= 1) = 0;
};

class Circle: public Shape
{
public:
	explicit Circle(int times_ = 1): m_times(times_) 
	{}
	
	static std::shared_ptr<Circle> Create(int times_ = 1)
	{
		return std::shared_ptr<Circle>(new Circle(times_));
	}
	
	virtual void Draw(int times_ = 1) 
	{
		std::cout << "Circle draw " << m_times << std::endl;
	}
private:
	int m_times;
};

class Triangle: public Shape
{
public:
	explicit Triangle(int times_ = 1): m_times(times_) 
	{}
	
	static std::shared_ptr<Triangle> Create(int times_ = 1)
	{
		return std::shared_ptr<Triangle>(new Triangle(times_));
	}
	
	virtual void Draw(int times_ = 1)
	{
		std::cout << "Triangle draw " << m_times << std::endl;
	}
private:
	int m_times;
};

class Rectangle: public Shape
{
public:
	explicit Rectangle(int times_ = 1): m_times(times_) 
	{}
	
	static std::shared_ptr<Rectangle> Create(int times_ = 1)
	{
		return std::shared_ptr<Rectangle>(new Rectangle(times_));
	}
	
	virtual void Draw(int times_ = 1)
	{
		std::cout << "Rectangle draw " << m_times << std::endl;
	}
private:
	int m_times;
};

int main(void)
{
    //using CreateFunc = std::function<std::shared_ptr<BASE>(ARGS)>;
    Factory<Shape, std::string, int> *factory = Singleton
    <Factory<Shape, std::string, int>>::GetInstance();
    
    factory->Add("circle", Circle::Create);
    factory->Add("triangle", Triangle::Create);
    factory->Add("rectangle", Rectangle::Create);
    
    std::string input; 
    while (1)
    {
    	std::getline(std::cin, input);
    	
    	if (input == "q")
    	{
    		break;
    	}
    	
	    try
	    {
			std::shared_ptr<Shape> shape = factory->Create(input, 2);
			shape->Draw();
	    }
	    catch(...)
	    {
	    	std::cout << "unsupported key delivered : " << input << std::endl <<
	    	"try again a valid key or 'q' to quit\n";
	    }
    }
	return 0;
}
