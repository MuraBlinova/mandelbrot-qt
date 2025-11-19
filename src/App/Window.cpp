#include "Window.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QLabel>
#include <QSlider>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>

#include <array>
#include <cmath>

namespace
{

constexpr std::array<GLfloat, 8u> vertices = {
	-1.0f, -1.0f,  // Bottom-left
	 1.0f, -1.0f,  // Bottom-right
	 1.0f,  1.0f,  // Top-right
	-1.0f,  1.0f   // Top-left
};

constexpr std::array<GLuint, 6u> indices = {
	0, 1, 2,
	0, 2, 3
};

}// namespace

Window::Window() noexcept
{
	const auto formatFPS = [](const auto value) {
		return QString("FPS: %1").arg(QString::number(value));
	};

	auto fps = new QLabel(formatFPS(0), this);
	fps->setStyleSheet("QLabel { color : white; }");

	auto iterLayout = new QHBoxLayout();
	auto iterLabelText = new QLabel("Iterations:", this);
	iterLabelText->setStyleSheet("QLabel { color : white; }");
	iterationsLabel_ = new QLabel(QString::number(maxIterations_), this);
	iterationsLabel_->setStyleSheet("QLabel { color : white; }");
	iterationsSlider_ = new QSlider(Qt::Horizontal, this);
	iterationsSlider_->setMinimum(10);
	iterationsSlider_->setMaximum(1000);
	iterationsSlider_->setValue(maxIterations_);
	iterationsSlider_->setStyleSheet("QSlider { background: transparent; }");
	
	iterLayout->addWidget(iterLabelText);
	iterLayout->addWidget(iterationsSlider_);
	iterLayout->addWidget(iterationsLabel_);


	auto layout = new QVBoxLayout();
	layout->addWidget(fps, 0);
	layout->addLayout(iterLayout, 0);
	layout->addStretch(1);

	setLayout(layout);

	timer_.start();

	connect(this, &Window::updateUI, [=] {
		fps->setText(formatFPS(ui_.fps));
	});
	
	connect(iterationsSlider_, &QSlider::valueChanged, this, &Window::onMaxIterationsChanged);
}

Window::~Window()
{
	{
		// Free resources with context bounded.
		const auto guard = bindContext();
		program_.reset();
	}
}

void Window::onInit()
{
	// Configure shaders
	program_ = std::make_unique<QOpenGLShaderProgram>(this);
	program_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/mandelbrot.vs");
	program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/mandelbrot.fs");
	program_->link();

	// Create VAO object
	vao_.create();
	vao_.bind();

	// Create VBO
	vbo_.create();
	vbo_.bind();
	vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo_.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(GLfloat)));

	// Create IBO
	ibo_.create();
	ibo_.bind();
	ibo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	ibo_.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(GLuint)));

	// Bind attributes
	program_->bind();

	program_->enableAttributeArray(0);
	program_->setAttributeBuffer(0, GL_FLOAT, 0, 2, static_cast<int>(2 * sizeof(GLfloat)));

	u_centerX_ = program_->uniformLocation("u_centerX");
	u_centerY_ = program_->uniformLocation("u_centerY");
	u_zoom_ = program_->uniformLocation("u_zoom");
	u_maxIterations_ = program_->uniformLocation("u_maxIterations");
	u_resolution_ = program_->uniformLocation("u_resolution");

	// Release all
	program_->release();
	vao_.release();
	ibo_.release();
	vbo_.release();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Clear all FBO buffers
	glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onRender()
{
	const auto guard = captureMetrics();

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT);

	// Bind VAO and shader program
	program_->bind();
	vao_.bind();

	program_->setUniformValue(u_centerX_, static_cast<float>(centerX_));
	program_->setUniformValue(u_centerY_, static_cast<float>(centerY_));
	program_->setUniformValue(u_zoom_, static_cast<float>(zoom_));
	program_->setUniformValue(u_maxIterations_, maxIterations_);
	program_->setUniformValue(u_resolution_, static_cast<float>(windowWidth_), static_cast<float>(windowHeight_));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	// Release VAO and shader program
	vao_.release();
	program_->release();

	++frameCount_;
}

void Window::onResize(const size_t width, const size_t height)
{
	// Configure viewport
	glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
	
	windowWidth_ = width;
	windowHeight_ = height;
	
	update();
}

Window::PerfomanceMetricsGuard::PerfomanceMetricsGuard(std::function<void()> callback)
	: callback_{ std::move(callback) }
{
}

Window::PerfomanceMetricsGuard::~PerfomanceMetricsGuard()
{
	if (callback_)
	{
		callback_();
	}
}

auto Window::captureMetrics() -> PerfomanceMetricsGuard
{
	return PerfomanceMetricsGuard{
		[&] {
			if (timer_.elapsed() >= 1000)
			{
				const auto elapsedSeconds = static_cast<float>(timer_.restart()) / 1000.0f;
				ui_.fps = static_cast<size_t>(std::round(frameCount_ / elapsedSeconds));
				frameCount_ = 0;
				emit updateUI();
			}
		}
	};
}

void Window::wheelEvent(QWheelEvent* event)
{
    const double zoomFactor = 1.1;
    const double oldZoom = zoom_;
    
    const QPointF mousePos = event->position();
    const double aspect = static_cast<double>(windowWidth_) / windowHeight_;
    
    const double sizeX = 2.0 / oldZoom;
    const double sizeY = sizeX / aspect;
    
    const double normX = (mousePos.x() / windowWidth_) * 2.0 - 1.0;
    const double normY = -((mousePos.y() / windowHeight_) * 2.0 - 1.0);
    
    const double mouseComplexX = centerX_ + normX * sizeX;
    const double mouseComplexY = centerY_ + normY * sizeY;
    
    if (event->angleDelta().y() > 0) {
        zoom_ *= zoomFactor;
    } else {
        zoom_ /= zoomFactor;
    }
    
    const double newSizeX = 2.0 / zoom_;
    const double newSizeY = newSizeX / aspect;
    
    centerX_ = mouseComplexX - normX * newSizeX;
    centerY_ = mouseComplexY - normY * newSizeY;
    
    update();
}

void Window::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDragging_ = true;
		lastMousePos_ = event->pos();
		setCursor(Qt::ClosedHandCursor);
	}
	event->accept();
}

void Window::mouseMoveEvent(QMouseEvent* event)
{
    if (isDragging_)
    {
        const QPoint delta = event->pos() - lastMousePos_;
        lastMousePos_ = event->pos();
        
        const double aspect = static_cast<double>(windowWidth_) / static_cast<double>(windowHeight_);
        const double sizeX = 2.0 / zoom_;
        const double sizeY = sizeX / aspect;
        
        const double speedMultiplier = 4.0;
        
        const double deltaX = -(delta.x() / static_cast<double>(windowWidth_)) * sizeX * speedMultiplier;
        const double deltaY = (delta.y() / static_cast<double>(windowHeight_)) * sizeY * speedMultiplier;
        
        centerX_ += deltaX;
        centerY_ += deltaY;
        
        update();
    }
    event->accept();
}

void Window::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		isDragging_ = false;
		setCursor(Qt::ArrowCursor);
	}
	event->accept();
}

void Window::onMaxIterationsChanged(int value)
{
	maxIterations_ = value;
	iterationsLabel_->setText(QString::number(value));
	update();
}