#pragma once
#include "search_server.h"
#include <vector>

#include <iostream>

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);

class JoinedDocuments {

    class BasicIterator {
        friend class JoinedDocuments;

    public:
        // Категория итератора - forward iterator
        // (итератор, который поддерживает операции инкремента и многократное разыменование)
        using iterator_category = std::forward_iterator_tag;
        // Тип элементов, по которым перемещается итератор
        using value_type = Document;
        // Тип, используемый для хранения смещения между итераторами
        using difference_type = std::ptrdiff_t;
        // Тип указателя на итерируемое значение
        using pointer = Document*;
        // Тип ссылки на итерируемое значение
        using reference = Document&;

        BasicIterator() {
            documents_ = nullptr;
            vectorNum_ = -1;
            documentNum_ = -1;
        }

        BasicIterator(std::vector<std::vector<Document>>* documents, size_t vectorNum, size_t documentNum)
            : vectorNum_(vectorNum),
            documentNum_(documentNum)
        {
            documents_ = documents;
            checkNotEmptyVector();
        }

        // Конвертирующий конструктор/конструктор копирования
        BasicIterator(const BasicIterator& other) noexcept
            : 
            vectorNum_(other.vectorNum_),
            documentNum_(other.documentNum_)
        {
            documents_ = other.documents_;
            checkNotEmptyVector();
        }

        // Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
        // пользовательского конструктора копирования, явно объявим оператор = и
        // попросим компилятор сгенерировать его за нас.
        BasicIterator& operator=(const BasicIterator& rhs) 
        {
            documents_ = rhs.documents_;
            vectorNum_ = rhs.vectorNum_;
            documentNum_ = rhs.documentNum_;
            return *this;
        }   

        // Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
        // Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
        [[nodiscard]] bool operator==(const BasicIterator& rhs) const noexcept {
            return documents_ == rhs.documents_ && vectorNum_ == rhs.vectorNum_ && documentNum_ == rhs.documentNum_;
        }

        // Оператор проверки итераторов на неравенство
        // Противоположен !=
        [[nodiscard]] bool operator!=(const BasicIterator& rhs) const noexcept {
            return !(*this == rhs);
        }

        // Оператор прединкремента. После его вызова итератор указывает на следующий элемент списка
        // Возвращает ссылку на самого себя
        // Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
        BasicIterator& operator++() noexcept {
            findNextNotEmpty();
            return *this;
        }

        // Оператор постинкремента. После его вызова итератор указывает на следующий элемент списка.
        // Возвращает прежнее значение итератора
        // Инкремент итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        BasicIterator operator++(int) noexcept {
            BasicIterator old_value(*this);
            ++(*this);
            return old_value;
        }

        // Операция разыменования. Возвращает ссылку на текущий элемент
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] reference operator*() const noexcept {
            return documents_->at(vectorNum_).at(documentNum_);
        }

        // Операция доступа к члену класса. Возвращает указатель на текущий элемент списка.
        // Вызов этого оператора у итератора, не указывающего на существующий элемент списка,
        // приводит к неопределённому поведению
        [[nodiscard]] pointer operator->() const noexcept {
            return &(documents_->at(vectorNum_).at(documentNum_));
        }

    private:
        size_t vectorNum_ = 0;
        size_t documentNum_ = 0;
        std::vector<std::vector<Document>>* documents_;

        void findNextNotEmpty() {
            if (documentNum_ + 1 < (*documents_)[vectorNum_].size()) {
                ++documentNum_;
            }
            else {
                vectorNum_++;
                //find first not empty vector
                while (vectorNum_ < documents_->size() && documents_->at(vectorNum_).empty()) { 
                    ++vectorNum_; 
                }
                //if not find
                if (vectorNum_ == documents_->size()) {
                    *this = BasicIterator();
                }
                else {
                    documentNum_ = 0;
                }
            }
        }

        void checkNotEmptyVector() {
            if ((*documents_)[vectorNum_].empty()) {
                findNextNotEmpty();
            }
        }
    };

public:
    using value_type = Document;
    using reference = value_type&;
    using const_reference = const value_type&;

    using Iterator = BasicIterator;
    using ConstIterator = const BasicIterator;


    explicit JoinedDocuments(std::vector<std::vector<Document>>&& documents)
        :documents_(documents)  {
    }
    
    Iterator begin() noexcept {
        return Iterator(&documents_, 0, 0);
    }

    Iterator end() {
        return Iterator();
    }

private:
    std::vector<std::vector<Document>> documents_;
};

JoinedDocuments ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);
